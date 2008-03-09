/***************************************************************************

    dsp56k.c
    Core implementation for the portable DSP56k emulator.
    Written by Andrew Gardner

****************************************************************************

    Note:

    This CPU emulator is very much a work-in-progress.  Thus far, it appears to be
    complete enough to run the memory tests for Polygonet Commanders.

    Some particularly WIP-like features of this core are as follows :
     * I ask many questions about my code throughout the core
     * The BITS(bits,op) macro is fine for a disassembler, but VERY slow for the
         inner loops of an executing core.  This will go away someday

***************************************************************************/

#include "debugger.h"
#include "deprecat.h"
#include "dsp56k.h"

// #define PC_E000

/***************************************************************************
    MACROS
***************************************************************************/

// ??? Are there namespace collision issues with just defining something "PC" ???
//     ...doesn't seem like it, but one never knows...

// Register macros
#define PC		dsp56k.pcuProgramCounter
#define SR		dsp56k.pcuStatus
#define OMR		dsp56k.pcuOperatingModeReg
#define SP		dsp56k.pcuStackPointer
#define LA		dsp56k.pcuLoopAddressReg
#define LC		dsp56k.pcuLoopCounter

#define SSH		dsp56k.pcuSystemStack[SP].w.h
#define SSL		dsp56k.pcuSystemStack[SP].w.l


#define X		dsp56k.aluDataRegs[0].d
#define X1		dsp56k.aluDataRegs[0].w.h
#define X0		dsp56k.aluDataRegs[0].w.l
#define Y		dsp56k.aluDataRegs[1].d
#define Y1		dsp56k.aluDataRegs[1].w.h
#define Y0		dsp56k.aluDataRegs[1].w.l

#define A		dsp56k.aluAccumRegs[0].lw
#define A2		dsp56k.aluAccumRegs[0].b.h4
#define A1		dsp56k.aluAccumRegs[0].w.h
#define A0		dsp56k.aluAccumRegs[0].w.l
#define B		dsp56k.aluAccumRegs[1].lw
#define B2		dsp56k.aluAccumRegs[1].b.h4
#define B1		dsp56k.aluAccumRegs[1].w.h
#define B0		dsp56k.aluAccumRegs[1].w.l

#define R0		dsp56k.aguAddressRegs[0]
#define R1		dsp56k.aguAddressRegs[1]
#define R2		dsp56k.aguAddressRegs[2]
#define R3		dsp56k.aguAddressRegs[3]

#define N0		dsp56k.aguOffsetRegs[0]
#define N1		dsp56k.aguOffsetRegs[1]
#define N2		dsp56k.aguOffsetRegs[2]
#define N3		dsp56k.aguOffsetRegs[3]

#define M0		dsp56k.aguModifierRegs[0]
#define M1		dsp56k.aguModifierRegs[1]
#define M2		dsp56k.aguModifierRegs[2]
#define M3		dsp56k.aguModifierRegs[3]

#define TEMP	dsp56k.aguTempReg
#define STATUS	dsp56k.aguStatusReg

// The CPU Stack
#define ST0		dsp56k.pcuSystemStack[0].d
#define ST1		dsp56k.pcuSystemStack[1].d
#define ST2		dsp56k.pcuSystemStack[2].d
#define ST3		dsp56k.pcuSystemStack[3].d
#define ST4		dsp56k.pcuSystemStack[4].d
#define ST5		dsp56k.pcuSystemStack[5].d
#define ST6		dsp56k.pcuSystemStack[6].d
#define ST7		dsp56k.pcuSystemStack[7].d
#define ST8		dsp56k.pcuSystemStack[8].d
#define ST9		dsp56k.pcuSystemStack[9].d
#define ST10	dsp56k.pcuSystemStack[10].d
#define ST11	dsp56k.pcuSystemStack[11].d
#define ST12	dsp56k.pcuSystemStack[12].d
#define ST13	dsp56k.pcuSystemStack[13].d
#define ST14	dsp56k.pcuSystemStack[14].d
#define ST15	dsp56k.pcuSystemStack[15].d
// !!! Is there really only 15 of them, or is there 16 ???

// Other
#define OP		dsp56k.op


// Peripheral RAM id's and addresses
#define PBCa   (0xffc0-0xffc0)
#define PCCa   (0xffc1-0xffc0)
#define PBDDRa (0xffc2-0xffc0)
#define PCDDRa (0xffc3-0xffc0)
#define PBC    (dsp56k_peripheral_ram[PBCa])
#define PCC    (dsp56k_peripheral_ram[PCCa])
#define PBDDR  (dsp56k_peripheral_ram[PBDDRa])
#define PCDDR  (dsp56k_peripheral_ram[PCDDRa])

#define HCRa (0xffc4-0xffc0)
#define HCR  (dsp56k_peripheral_ram[HCRa])

#define COCRa (0xffc8-0xffc0)
#define COCR  (dsp56k_peripheral_ram[COCR])

#define CRASSI0a (0xffd0-0xffc0)
#define CRBSSI0a (0xffd1-0xffc0)
#define CRASSI0  (dsp56k_peripheral_ram[CRASSI0a])
#define CRBSSI0  (dsp56k_peripheral_ram[CRBSSI0a])

#define CRASSI1a (0xffd8-0xffc0)
#define CRBSSI1a (0xffd9-0xffc0)
#define CRASSI1  (dsp56k_peripheral_ram[CRASSI1a])
#define CRBSSI1  (dsp56k_peripheral_ram[CRBSSI1a])

#define PLCRa (0xffdc-0xffc0)
#define PLCR  (dsp56k_peripheral_ram[PLCRa])

#define BCRa (0xffde-0xffc0)
#define IPRa (0xffdf-0xffc0)
#define BCR  (dsp56k_peripheral_ram[BCRa])
#define IPR  (dsp56k_peripheral_ram[IPRa])

#define PBDa (0xffe2-0xffc0)
#define PCDa (0xffe3-0xffc0)
#define HSRa (0xffe4-0xffc0)
#define PBD  (dsp56k_peripheral_ram[PBDa])
#define PCD  (dsp56k_peripheral_ram[PCDa])
#define HSR  (dsp56k_peripheral_ram[HSRa])

#define HTXHRXa (0xffe5-0xffc0)
#define HTXHRX  (dsp56k_peripheral_ram[HTXHRXa])

#define COSRa   (0xffe8-0xffc0)
#define CRXCTXa (0xffe9-0xffc0)
#define COSR    (dsp56k_peripheral_ram[COSRa])
#define CRXCTX  (dsp56k_peripheral_ram[CRXCTXa])

#define TCRa  (0xffec-0xffc0)
#define TCTRa (0xffed-0xffc0)
#define TCPRa (0xffee-0xffc0)
#define TPRa  (0xffef-0xffc0)
#define TCR   (dsp56k_peripheral_ram[TCRa])
#define TCTR  (dsp56k_peripheral_ram[TCTRa])
#define TCPR  (dsp56k_peripheral_ram[TCPRa])
#define TPR   (dsp56k_peripheral_ram[TPRa])

#define TSRSSI0a (0xfff0-0xffc0)
#define TRXSSI0a (0xfff1-0xffc0)
#define RSMA0a   (0xfff2-0xffc0)
#define RSMB0a   (0xfff3-0xffc0)
#define TSMA0a   (0xfff4-0xffc0)
#define TSMB0a   (0xfff5-0xffc0)
#define TSRSSI0  (dsp56k_peripheral_ram[TSRSSI0a])
#define TRXSSI0  (dsp56k_peripheral_ram[TRXSSI0a])
#define RSMA0    (dsp56k_peripheral_ram[RSMA0a])
#define RSMB0    (dsp56k_peripheral_ram[RSMB0a])
#define TSMA0    (dsp56k_peripheral_ram[TSMA0a])
#define TSMB0    (dsp56k_peripheral_ram[TSMB0a])

#define TSRSSI1a (0xfff8-0xffc0)
#define TRXSSI1a (0xfff9-0xffc0)
#define RSMA1a   (0xfffa-0xffc0)
#define RSMB1a   (0xfffb-0xffc0)
#define TSMA1a   (0xfffc-0xffc0)
#define TSMB1a   (0xfffd-0xffc0)
#define TSRSSI1  (dsp56k_peripheral_ram[TSRSSI1a])
#define TRXSSI1  (dsp56k_peripheral_ram[TRXSSI1a])
#define RSMA1    (dsp56k_peripheral_ram[RSMA1a])
#define RSMB1    (dsp56k_peripheral_ram[RSMB1a])
#define TSMA1    (dsp56k_peripheral_ram[TSMA1a])
#define TSMB1    (dsp56k_peripheral_ram[TSMB1a])



// Status Register Bits
#define lfBIT ((SR & 0x8000) != 0)
#define fvBIT ((SR & 0x4000) != 0)
#define s1BIT ((SR & 0x0800) != 0)
#define s0BIT ((SR & 0x0400) != 0)
#define i1BIT ((SR & 0x0200) != 0)
#define i0BIT ((SR & 0x0100) != 0)
#define sBIT  ((SR & 0x0080) != 0)
#define lBIT  ((SR & 0x0040) != 0)
#define eBIT  ((SR & 0x0020) != 0)
#define uBIT  ((SR & 0x0010) != 0)
#define nBIT  ((SR & 0x0008) != 0)
#define zBIT  ((SR & 0x0004) != 0)
#define vBIT  ((SR & 0x0002) != 0)
#define cBIT  ((SR & 0x0001) != 0)

#define CLEAR_lfBIT() (SR &= (~0x8000))
#define CLEAR_fvBIT() (SR &= (~0x4000))
#define CLEAR_s1BIT() (SR &= (~0x0800))
#define CLEAR_s0BIT() (SR &= (~0x0400))
#define CLEAR_i1BIT() (SR &= (~0x0200))
#define CLEAR_i0BIT() (SR &= (~0x0100))
#define CLEAR_sBIT()  (SR &= (~0x0080))
#define CLEAR_lBIT()  (SR &= (~0x0040))
#define CLEAR_eBIT()  (SR &= (~0x0020))
#define CLEAR_uBIT()  (SR &= (~0x0010))
#define CLEAR_nBIT()  (SR &= (~0x0008))
#define CLEAR_zBIT()  (SR &= (~0x0004))
#define CLEAR_vBIT()  (SR &= (~0x0002))
#define CLEAR_cBIT()  (SR &= (~0x0001))

#define SET_lfBIT() (SR |= 0x8000)
#define SET_fvBIT() (SR |= 0x4000)
#define SET_s1BIT() (SR |= 0x0800)
#define SET_s0BIT() (SR |= 0x0400)
#define SET_i1BIT() (SR |= 0x0200)
#define SET_i0BIT() (SR |= 0x0100)
#define SET_sBIT()  (SR |= 0x0080)
#define SET_lBIT()  (SR |= 0x0040)
#define SET_eBIT()  (SR |= 0x0020)
#define SET_uBIT()  (SR |= 0x0010)
#define SET_nBIT()  (SR |= 0x0008)
#define SET_zBIT()  (SR |= 0x0004)
#define SET_vBIT()  (SR |= 0x0002)
#define SET_cBIT()  (SR |= 0x0001)



// Stack Pointer Bits
#define ufBIT  ((SP & 0x20) != 0)
#define seBIT  ((SP & 0x10) != 0)

#define CLEAR_ufBIT() (SP &= (~0x20))
#define CLEAR_seBIT() (SP &= (~0x10))

#define SET_ufBIT() (SP |= 0x20)
#define SET_seBIT() (SP |= 0x10)



// Operating Mode Register Bits
#define cdBIT	((OMR & 0x80) != 0)
#define sdBIT	((OMR & 0x40) != 0)
#define  rBIT	((OMR & 0x20) != 0)
#define saBIT	((OMR & 0x10) != 0)
#define mcBIT	((OMR & 0x04) != 0)
#define mbBIT	((OMR & 0x02) != 0)
#define maBIT	((OMR & 0x01) != 0)

#define CLEAR_cdBIT() (OMR &= (~0x80))
#define CLEAR_sdBIT() (OMR &= (~0x40))
#define CLEAR_rBIT()  (OMR &= (~0x20))
#define CLEAR_saBIT() (OMR &= (~0x10))
#define CLEAR_mcBIT() (OMR &= (~0x04))
#define CLEAR_mbBIT() (OMR &= (~0x02))
#define CLEAR_maBIT() (OMR &= (~0x01))

#define SET_cdBIT() (OMR |= 0x80)
#define SET_sdBIT() (OMR |= 0x40)
#define SET_rBIT()  (OMR |= 0x20)
#define SET_saBIT() (OMR |= 0x10)
#define SET_mcBIT() (OMR |= 0x04)
#define SET_mbBIT() (OMR |= 0x02)
#define SET_maBIT() (OMR |= 0x01)


// Interrupt Priority Register Bits
#define tl1BIT   ((IPR & 0x8000) != 0)
#define tl0BIT   ((IPR & 0x4000) != 0)
#define s1l1BIT  ((IPR & 0x2000) != 0)
#define s1l0BIT  ((IPR & 0x1000) != 0)
#define s0l1BIT  ((IPR & 0x0800) != 0)
#define s0l0BIT  ((IPR & 0x0400) != 0)
#define hl1BIT   ((IPR & 0x0200) != 0)
#define hl0BIT   ((IPR & 0x0100) != 0)
#define cl1BIT   ((IPR & 0x0080) != 0)
#define cl0BIT   ((IPR & 0x0040) != 0)
#define ibl2BIT  ((IPR & 0x0020) != 0)
#define ibl1BIT  ((IPR & 0x0010) != 0)
#define ibl0BIT  ((IPR & 0x0008) != 0)
#define ial2BIT  ((IPR & 0x0004) != 0)
#define ial1BIT  ((IPR & 0x0002) != 0)
#define ial0BIT  ((IPR & 0x0001) != 0)

#define CLEAR_tl1BIT()   (IPR &= (~0x8000))
#define CLEAR_tl0BIT()   (IPR &= (~0x4000))
#define CLEAR_s1l1BIT()  (IPR &= (~0x2000))
#define CLEAR_s1l0BIT()  (IPR &= (~0x1000))
#define CLEAR_s0l1BIT()  (IPR &= (~0x0800))
#define CLEAR_s0l0BIT()  (IPR &= (~0x0400))
#define CLEAR_hl1BIT()   (IPR &= (~0x0200))
#define CLEAR_hl0BIT()   (IPR &= (~0x0100))
#define CLEAR_cl1BIT()   (IPR &= (~0x0080))
#define CLEAR_cl0BIT()   (IPR &= (~0x0040))
#define CLEAR_ibl2BIT()  (IPR &= (~0x0020))
#define CLEAR_ibl1BIT()  (IPR &= (~0x0010))
#define CLEAR_ibl0BIT()  (IPR &= (~0x0008))
#define CLEAR_ial2BIT()  (IPR &= (~0x0004))
#define CLEAR_ial1BIT()  (IPR &= (~0x0002))
#define CLEAR_ial0BIT()  (IPR &= (~0x0001))

#define SET_tl1BIT()   (IPR |= 0x8000)
#define SET_tl0BIT()   (IPR |= 0x4000)
#define SET_s1l1BIT()  (IPR |= 0x2000)
#define SET_s1l0BIT()  (IPR |= 0x1000)
#define SET_s0l1BIT()  (IPR |= 0x0800)
#define SET_s0l0BIT()  (IPR |= 0x0400)
#define SET_hl1BIT()   (IPR |= 0x0200)
#define SET_hl0BIT()   (IPR |= 0x0100)
#define SET_cl1BIT()   (IPR |= 0x0080)
#define SET_cl0BIT()   (IPR |= 0x0040)
#define SET_ibl2BIT()  (IPR |= 0x0020)
#define SET_ibl1BIT()  (IPR |= 0x0010)
#define SET_ibl0BIT()  (IPR |= 0x0008)
#define SET_ial2BIT()  (IPR |= 0x0004)
#define SET_ial1BIT()  (IPR |= 0x0002)
#define SET_ial0BIT()  (IPR |= 0x0001)



// Bus Control Register Bits
#define rhBIT ((BCR & 0x8000) != 0)
#define bsBIT ((BCR & 0x4000) != 0)

#define CLEAR_rhBIT() (BCR &= (~0x8000))
#define CLEAR_bsBIT() (BCR &= (~0x4000))

#define SET_rhBIT() (BCR |= 0x8000)
#define SET_bsBIT() (BCR |= 0x4000)



// Port B Control Register Bits
#define bcBIT ((PBC & 0x0001) != 0)

#define CLEAR_bcBIT() (PBC &= (~0x0001))

#define SET_bcBIT() (PBC |= 0x0001)



// HOST INTERFACE (dsp56k side)
// Read/Write Host Control Register Bits
#define hf3BIT  ((HCR & 0x0010) != 0)
#define hf2BIT  ((HCR & 0x0008) != 0)
#define hcieBIT ((HCR & 0x0004) != 0)
#define htieBIT ((HCR & 0x0002) != 0)
#define hrieBIT ((HCR & 0x0001) != 0)

#define CLEAR_hf3BIT()  (HCR &= (~0x0010))
#define CLEAR_hf2BIT()  (HCR &= (~0x0008))
#define CLEAR_hcieBIT() (HCR &= (~0x0004))
#define CLEAR_htieBIT() (HCR &= (~0x0002))
#define CLEAR_hrieBIT() (HCR &= (~0x0001))

#define SET_hf3BIT()  (HCR |= 0x0010)
#define SET_hf2BIT()  (HCR |= 0x0008)
#define SET_hcieBIT() (HCR |= 0x0004)
#define SET_htieBIT() (HCR |= 0x0002)
#define SET_hrieBIT() (HCR |= 0x0001)



// Read-only Host Status Register Bits
#define dmaBIT  ((HSR & 0x0080) != 0)
#define hf1BIT  ((HSR & 0x0010) != 0)
#define hf0BIT  ((HSR & 0x0008) != 0)
#define hcpBIT  ((HSR & 0x0004) != 0)
#define htdeBIT ((HSR & 0x0002) != 0)
#define hrdfBIT ((HSR & 0x0001) != 0)

#define CLEAR_dmaBIT()  (HSR &= (~0x0080))
#define CLEAR_hf1BIT()  (HSR &= (~0x0010))
#define CLEAR_hf0BIT()  (HSR &= (~0x0008))
#define CLEAR_hcpBIT()  (HSR &= (~0x0004))
#define CLEAR_htdeBIT() (HSR &= (~0x0002))
#define CLEAR_hrdfBIT() (HSR &= (~0x0001))

#define SET_dmaBIT()  (HSR |= 0x0080)
#define SET_hf1BIT()  (HSR |= 0x0010)
#define SET_hf0BIT()  (HSR |= 0x0008)
#define SET_hcpBIT()  (HSR |= 0x0004)
#define SET_htdeBIT() (HSR |= 0x0002)
#define SET_hrdfBIT() (HSR |= 0x0001)



// HOST INTERFACE (host side)
// Interrupt Control Register Bits
#define x_initBIT ((dsp56k.HI.ICR & 0x0080) != 0)
#define x_hm1BIT  ((dsp56k.HI.ICR & 0x0040) != 0)
#define x_hm0BIT  ((dsp56k.HI.ICR & 0x0020) != 0)
#define x_hf1BIT  ((dsp56k.HI.ICR & 0x0010) != 0)
#define x_hf0BIT  ((dsp56k.HI.ICR & 0x0008) != 0)
#define x_treqBIT ((dsp56k.HI.ICR & 0x0002) != 0)
#define x_rreqBIT ((dsp56k.HI.ICR & 0x0001) != 0)

#define CLEAR_x_initBIT() (dsp56k.HI.ICR &= (~0x0080))
#define CLEAR_x_hm1BIT()  (dsp56k.HI.ICR &= (~0x0040))
#define CLEAR_x_hm0BIT()  (dsp56k.HI.ICR &= (~0x0020))
#define CLEAR_x_hf1BIT()  (dsp56k.HI.ICR &= (~0x0010))
#define CLEAR_x_hf0BIT()  (dsp56k.HI.ICR &= (~0x0008))
#define CLEAR_x_treqBIT() (dsp56k.HI.ICR &= (~0x0002))
#define CLEAR_x_rreqBIT() (dsp56k.HI.ICR &= (~0x0001))

#define SET_x_initBIT() (dsp56k.HI.ICR |= 0x0080)
#define SET_x_hm1BIT()  (dsp56k.HI.ICR |= 0x0040)
#define SET_x_hm0BIT()  (dsp56k.HI.ICR |= 0x0020)
#define SET_x_hf1BIT()  (dsp56k.HI.ICR |= 0x0010)
#define SET_x_hf0BIT()  (dsp56k.HI.ICR |= 0x0008)
#define SET_x_treqBIT() (dsp56k.HI.ICR |= 0x0002)
#define SET_x_rreqBIT() (dsp56k.HI.ICR |= 0x0001)



// Command Vector Register Bit
#define x_hcBIT ((dsp56k.HI.CVR & 0x0080) != 0)

#define CLEAR_x_hcBIT() (dsp56k.HI.CVR &= (~0x0080))

#define SET_x_hcBIT() (dsp56k.HI.CVR |= 0x0080)



// Interrupt Status Register Bits
#define x_hreqBIT ((dsp56k.HI.ISR & 0x0080) != 0)
#define x_dmaBIT  ((dsp56k.HI.ISR & 0x0040) != 0)
#define x_hf3BIT  ((dsp56k.HI.ISR & 0x0010) != 0)
#define x_hf2BIT  ((dsp56k.HI.ISR & 0x0008) != 0)
#define x_trdyBIT ((dsp56k.HI.ISR & 0x0004) != 0)
#define x_txdeBIT ((dsp56k.HI.ISR & 0x0002) != 0)
#define x_rxdfBIT ((dsp56k.HI.ISR & 0x0001) != 0)

#define CLEAR_x_hreqBIT() (dsp56k.HI.ISR &= (~0x0080))
#define CLEAR_x_dmaBIT()  (dsp56k.HI.ISR &= (~0x0040))
#define CLEAR_x_hf3BIT()  (dsp56k.HI.ISR &= (~0x0010))
#define CLEAR_x_hf2BIT()  (dsp56k.HI.ISR &= (~0x0008))
#define CLEAR_x_trdyBIT() (dsp56k.HI.ISR &= (~0x0004))
#define CLEAR_x_txdeBIT() (dsp56k.HI.ISR &= (~0x0002))
#define CLEAR_x_rxdfBIT() (dsp56k.HI.ISR &= (~0x0001))

#define SET_x_hreqBIT() (dsp56k.HI.ISR |= 0x0080)
#define SET_x_dmaBIT()  (dsp56k.HI.ISR |= 0x0040)
#define SET_x_hf3BIT()  (dsp56k.HI.ISR |= 0x0010)
#define SET_x_hf2BIT()  (dsp56k.HI.ISR |= 0x0008)
#define SET_x_trdyBIT() (dsp56k.HI.ISR |= 0x0004)
#define SET_x_txdeBIT() (dsp56k.HI.ISR |= 0x0002)
#define SET_x_rxdfBIT() (dsp56k.HI.ISR |= 0x0001)



// Interrupt Vector Register Bits
#define x_iv7BIT ((dsp56k.HI.IVR & 0x0080) != 0)
#define x_iv6BIT ((dsp56k.HI.IVR & 0x0040) != 0)
#define x_iv5BIT ((dsp56k.HI.IVR & 0x0020) != 0)
#define x_iv4BIT ((dsp56k.HI.IVR & 0x0010) != 0)
#define x_iv3BIT ((dsp56k.HI.IVR & 0x0008) != 0)
#define x_iv2BIT ((dsp56k.HI.IVR & 0x0004) != 0)
#define x_iv1BIT ((dsp56k.HI.IVR & 0x0002) != 0)
#define x_iv0BIT ((dsp56k.HI.IVR & 0x0001) != 0)

#define CLEAR_x_iv7BIT() (dsp56k.HI.IVR &= (~0x0080))
#define CLEAR_x_iv6BIT() (dsp56k.HI.IVR &= (~0x0040))
#define CLEAR_x_iv5BIT() (dsp56k.HI.IVR &= (~0x0020))
#define CLEAR_x_iv4BIT() (dsp56k.HI.IVR &= (~0x0010))
#define CLEAR_x_iv3BIT() (dsp56k.HI.IVR &= (~0x0008))
#define CLEAR_x_iv2BIT() (dsp56k.HI.IVR &= (~0x0004))
#define CLEAR_x_iv1BIT() (dsp56k.HI.IVR &= (~0x0002))
#define CLEAR_x_iv0BIT() (dsp56k.HI.IVR &= (~0x0001))

#define SET_x_iv7BIT() (dsp56k.HI.IVR | 0x0080)
#define SET_x_iv6BIT() (dsp56k.HI.IVR | 0x0040)
#define SET_x_iv5BIT() (dsp56k.HI.IVR | 0x0020)
#define SET_x_iv4BIT() (dsp56k.HI.IVR | 0x0010)
#define SET_x_iv3BIT() (dsp56k.HI.IVR | 0x0008)
#define SET_x_iv2BIT() (dsp56k.HI.IVR | 0x0004)
#define SET_x_iv1BIT() (dsp56k.HI.IVR | 0x0002)
#define SET_x_iv0BIT() (dsp56k.HI.IVR | 0x0001)





// IRQ Interfaces
#define LINE_MODA  (dsp56k.irq_modA)
#define LINE_MODB  (dsp56k.irq_modB)
#define LINE_MODC  (dsp56k.irq_modC)
#define LINE_RESET (dsp56k.irq_reset)





/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

// DSP56156 Host Interface - page 94 of DSP56156UM
typedef struct
{
	// DSP56156 SIDE
	// Three words in the DSP processor?s address space
	// UINT8  HCR $FFC4
	// UINT8  HSR $FFE4
	// UINT16 HTX $FFE5 - same as below
	// UINT16 HRX $FFE5 - same as above

	// HOST SIDE
	// The HI appears as a memory mapped peripheral, occupying 8 bytes in the host processor?s address space
	UINT8 TXHRXH, TXLRXL;

	UINT8 ICR;
	UINT8 CVR;
	UINT8 ISR;
	UINT8 IVR;

	// control lines available to the host
	UINT8 HA0, HA1, HA2;
	UINT8 hatHRW;
	UINT8 hatHEN;
	UINT8 hatHREQ;
	UINT8 hatHACK;

} dsp56k_host_interface;


// DSP56156 Registers - sizes specific to chip
typedef struct
{
	// See section 1-22 in DSP56156UM.pdf for scrutinization...

	// PCU Registers
	UINT16			pcuProgramCounter;			//  PC
	UINT16			pcuStatus;					//  MR,CCR / SR
	UINT16			pcuLoopCounter;				//  LC
	UINT16			pcuLoopAddressReg;			//  LA
	UINT8			pcuStackPointer;			//  SP
	UINT8			pcuOperatingModeReg;		//  OMR
	PAIR			pcuSystemStack[16];			//  SSH,SSL (*15)

	// ALU Registers
	PAIR			aluDataRegs[2];				//  X1,X0     &   Y1,Y0
	PAIR64			aluAccumRegs[2];			//  A2,A1,A0  &   B2,B1,B0

	// AGU Registers
	UINT16          aguAddressRegs[4];			//  R0,R1,R2,R3
	UINT16          aguOffsetRegs[4];			//  N0,N1,N2,N3
	UINT16          aguModifierRegs[4];			//  M0,M1,M2,M3
	UINT16          aguTempReg;					//  TEMP
	UINT8           aguStatusReg;				//  Status


	// IRQ lines
	UINT8           irq_modA;					//  aka IRQA - can be defined edge or level sensitive (though i'm not sure how)
	UINT8           irq_modB;					//  aka IRQA - can be defined edge or level sensitive (though i'm not sure how)
	UINT8           irq_modC;					//  just modC :)
	UINT8           irq_reset;					//  Always level-sensitive

	int		(*irq_callback)(int irqline);

	// Internal Stuff
	UINT32			ppc;						// Previous PC - for debugger
	UINT16			op;							// Current opcode
	int				interrupt_cycles;

	int				repFlag;					// Knowing if we're in a 'repeat' state (dunno how the processor does this)
	UINT32			repAddr;					// The address of the instruction to repeat...

	// Interfaces - ports B and C *can* be these if everything's setup right.
	dsp56k_host_interface HI;

	const void *	config;

} dsp56k_regs;

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void dsp56k_reset(void);


/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static dsp56k_regs dsp56k;
static int dsp56k_icount;

static UINT16 hack_memory_offset = 0;
static UINT16 *dsp56k_peripheral_ram;
static UINT16 *dsp56k_program_ram;

/***************************************************************************
    IRQ HANDLING
***************************************************************************/

static void check_irqs(void)
{

}


static void set_irq_line(int irqline, int state)
{
	if (irqline == 3)
	{
		LINE_RESET = state;

		if(LINE_RESET != CLEAR_LINE)
		{
			int irq_vector = (*dsp56k.irq_callback)(3);

			PC = irq_vector;

			LINE_RESET = CLEAR_LINE;
		}
	}
}



/***************************************************************************
    CONTEXT SWITCHING
***************************************************************************/

static void dsp56k_get_context(void *dst)
{
	/* copy the context */
	if (dst)
		*(dsp56k_regs *)dst = dsp56k;
}


static void dsp56k_set_context(void *src)
{
	/* copy the context */
	if (src)
		dsp56k = *(dsp56k_regs *)src;
	memory_set_opbase(PC);

	/* check for IRQs */
	check_irqs();
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static void dsp56k_init(int index, int clock, const void *_config, int (*irqcallback)(int))
{
	dsp56k.config = _config;
	dsp56k.irq_callback = irqcallback;
}


static void dsp56k_reset_HI(void)
{
	dsp56k.HI.CVR = 0x16;		// clears HC and sets HL
}

// Page 101 (7-25) in the Family Manual
static void dsp56k_reset(void)
{
	if (dsp56k.config == NULL)
	{
		memory_set_opbase(PC);

		// Handle internal stuff
		dsp56k.interrupt_cycles = 0;


		// Internal peripheral devices are reset, and pins revert to general I/O pins

		// Modifier registers are set
		M0 = M1 = M2 = M3 = 0xffff;

		// BCR is set - the really slow bootup mode & the Bus State status bit high (0x4xxx)
		//BCR = 0x43ff;

		// Stack pointer is cleared
		SP = 0x00;					// The docs say nothing about ufBIT & seBIT, but this should be right

		// Documentation says 'MR' is setup, but it really means 'SR' is setup
		SR = 0x0300;				// Only the Interrupt mask bits of the Status Register are set upon reset


		// !!! GO THROUGH AND GET ALL THESE RIGHT SOMEDAY !!!
		HSR = 0x0000;
		SET_htdeBIT();

		dsp56k_reset_HI();

		OMR = 0x00;					// All is cleared, except for the IRQ lines below
		IPR = 0x00;

		dsp56k.repFlag = 0;			// Certainly not repeating to start
		dsp56k.repAddr = 0x0000;	// Reset the address too...

		// Manipulate everything you need to for the ports (!! maybe these will be callbacks someday !!)...
		data_write_word_16le(0xffc0, 0x0000);	// Sets Port B Control Register to general I/O
		data_write_word_16le(0xffc2, 0x0000);	// Sets Port B Data Direction Register as input
		data_write_word_16le(0xffc1, 0x0000);	// Sets Port C Control Register to general I/O
		data_write_word_16le(0xffc3, 0x0000);	// Sets Port C Data Direction Register as input

		// Now that we're leaving, set ma, mb, and mc from MODA, MODB, and MODC lines
		// I believe polygonet sets everyone to mode 0...  The following reflects this...
		CLEAR_maBIT();
		CLEAR_mbBIT();

		// switch bootup sequence based on chip operating mode
		switch((mbBIT << 1) | maBIT)
		{
			// [Special Bootstrap 1] Bootstrap from an external byte-wide memory located at P:$c000
			case 0x0:
				PC = 0x0000; // 0x0030; // 0x0032; // 0x002e; // 0x0000; // 0x002c;
				break;

			// [Special Bootstrap 2] Bootstrap from the Host port or SSI0
			case 0x1:
				PC = 0x0000;
				break;

			// [Normal Expanded] Internal PRAM enabled; External reset at P:$e000
			case 0x2:
				PC = 0xe000;
				break;

			// [Development Expanded] Int. program memory disabled; Ext. reset at P:$0000
			case 0x3:
				PC = 0x0000;
				break;
		}
	}
	else
	{
		PC = *((UINT16*)dsp56k.config);
	}
}


static void dsp56k_exit(void)
{
}



/***************************************************************************
    CORE INCLUDE
***************************************************************************/

#define ROPCODE(pc)   cpu_readop16(pc)

#include "dsp56ops.c"



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

static int dsp56k_execute(int cycles)
{
	/* skip if halted */
	if (LINE_RESET)
		return cycles;

	dsp56k_icount = cycles;
	dsp56k_icount -= dsp56k.interrupt_cycles;
	dsp56k.interrupt_cycles = 0;

	while(dsp56k_icount > 0)
		execute_one();

	dsp56k_icount -= dsp56k.interrupt_cycles;
	dsp56k.interrupt_cycles = 0;

	return cycles - dsp56k_icount;
}



/***************************************************************************
    DISASSEMBLY HOOK
***************************************************************************/

extern offs_t dsp56k_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);



/****************************************************************************
 *  Internal Memory Handlers
 ****************************************************************************/

static READ16_HANDLER( peripheral_register_r )
{
//  logerror("peripheral_register_r 0x%x\n", offset+0xffc0);

	switch (offset)
	{
		case HCRa: //ffc4
			return HCR;
			break;

		case HSRa: //ffe4
			return HSR;
			break;

		case HTXHRXa: //ffe5
			// The HRX register contains valid data when the HRDF bit is set.
//          if (hrdfBIT)
			return HTXHRX;

			/* FIXME: Following code never reached due to commented if above */
#if 0
			// Reading HRX clears HRDF.
			CLEAR_hrdfBIT();

			// The DSP may program the HRIE bit to cause a Host Receive Data interrupt when HRDF is set.

			return 0x0000;
#endif
			break;
	}

	return dsp56k_peripheral_ram[offset];
}

static WRITE16_HANDLER( peripheral_register_w )
{
	COMBINE_DATA(&dsp56k_peripheral_ram[offset]);

	logerror("peripheral_register_w 0x%x 0x%x (@%x)\n", offset+0xffc0, data, PC);

	switch (offset)
	{
		case HCRa: //ffc4
			// The HCR register occupies the low order byte of the internal data bus -
			//   the high order portion is zero-filled.

			// Changing HF2 will change the Host Flag 2 (HF2) bit of the Interrupt Status
			//   Register ISR on the host processor side of the host interface.
			if (hf2BIT)
				SET_x_hf2BIT();
			else
				CLEAR_x_hf2BIT();

			// Changing HF3 will change the Host Flag 3 (HF3) bit of the Interrupt Status
			//   Register ISR on the host processor side of the host interface.
			if (hf3BIT)
			{
				SET_x_hf3BIT();
			}
			else
				CLEAR_x_hf3BIT();

			break;

		case HSRa: //ffe4
			/* READ ONLY */
			break;

		case HTXHRXa: //ffe5

			// ??? Can you write here even if you don't have the requirements?

			// Writing the HTX register clears HTDE (HSR bit 1)
			CLEAR_htdeBIT();

			// The DSP may program the HTIE (HCR bit 1) bit to cause a Host Transmit Data
			//   interrupt when HTDE is set

			// The HTX register is transferred as 16-bit data to the Receive Byte Registers
			//   RXH:RXL if both the HTDE (HSR bit 1) bit and the Receive Data Full,
			//   RXDF (ISR bit 0), status bit are cleared
//          printf("%d %d %x\n", htdeBIT, x_rxdfBIT, dsp56k_peripheral_ram[offset]);
			if ( !htdeBIT && !x_rxdfBIT )
			{
				dsp56k.HI.TXHRXH = (HTXHRX & 0xff00) >> 8;
				dsp56k.HI.TXLRXL = (HTXHRX & 0x00ff);

				// This transfer operation sets RXDF (ISR bit 0) and HTDE (HSR bit 1).
				SET_x_rxdfBIT();
			}
			break;
	}
}

UINT16 dsp56k_get_peripheral_memory(UINT16 addr)
{
	if (addr >= 0xffc0)		// && addr <= 0xffff
	{
		return dsp56k_peripheral_ram[addr-0xffc0];
	}
	else
	{
		logerror("DSP56k - Peripheral memory requested does not exist\n");
		return 0x00;
	}
}

/****************************************************************************
 *  Internal Memory Maps
 ****************************************************************************/

static ADDRESS_MAP_START( dsp56156_program_memory, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE(&dsp56k_program_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp56156_x_data_memory, ADDRESS_SPACE_DATA, 16 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0xffc0, 0xffff) AM_READWRITE(peripheral_register_r, peripheral_register_w) AM_BASE(&dsp56k_peripheral_ram)
ADDRESS_MAP_END


/**************************************************************************
 * Host Interface (HI) functionality.
 **************************************************************************/

void dsp56k_host_interface_write(UINT8 addr, UINT8 data)
{
	switch (addr)
	{
		case 0x0:			// ICR
			dsp56k.HI.ICR = data;

			// Changing HF0 also changes the Host Flag bit 0 (HF0) of the Host Status register HSR on the DSP side of the HI.
			if (x_hf0BIT) SET_hf0BIT();
			if (x_hf1BIT) SET_hf1BIT();

			break;

		case 0x1:			// CVR
			dsp56k.HI.CVR = data;

			// Normally the host processor sets HC=1 to request the host command
			//   exception from the DSP.
			if (dsp56k.HI.CVR & 0x80)
			{
				// Setting HC (bit 0x80) causes HCP (Host Command Pending) to be set in the HSR register. The host
				//   can write HC and HV in the same write cycle if desired. HC is cleared by DSP reset.
				SET_hcpBIT();

				// reset the pc to the proper address
				logerror("RESET (%04x) sent\n", (dsp56k.HI.CVR & 0x1f) << 1);
				PC = (dsp56k.HI.CVR & 0x1f) << 1;

				// When the host command exception is taken by the DSP, the HC
				//   bit is cleared by the HI hardware.
				CLEAR_x_hcBIT();
			}
			break;

		case 0x2: break;	// ISR - read only

		case 0x3:			// IVR
			// 68000 series communication
			break;

		case 0x4: break;	// Unused
		case 0x5: break;	// Unused

		case 0x6:			// TXH/RXH

			// Data may be written into the Transmit Byte Registers when the
			// Transmit Data Register Empty TXDE bit is set.
			if (!x_txdeBIT)
			{
				dsp56k.HI.TXHRXH = data;
			}

			break;

		case 0x7:			// TXL/RXL

			// Data may be written into the Transmit Byte Registers when the
			// Transmit Data Register Empty TXDE bit is set.
			if (!x_txdeBIT)
				dsp56k.HI.TXLRXL = data;

			// writing the Transmit Low register TXL clears the TXDE bit
			CLEAR_x_txdeBIT();


			// The Transmit Byte Registers TXH:TXL are transferred as 16-bit data to the Host Receive
			//   Data Register HRX when both TXDE bit and the Host Receive Data Full, HRDF, bit are
			//   cleared.
			if (!x_txdeBIT && !hrdfBIT)
			{
				HTXHRX = ( ((UINT16)dsp56k.HI.TXHRXH) << 8 ) | (UINT16)dsp56k.HI.TXLRXL;

				// !!! Hack !!! Move it straight to program memory...
				dsp56k_program_ram[hack_memory_offset] = HTXHRX;
				logerror("Wrote memoryOffset[%d] : %04x\n", hack_memory_offset, dsp56k_program_ram[hack_memory_offset]);
				hack_memory_offset++;

				// This transfer operation sets TXDE and HRDF.
//              SET_x_txdeBIT();
//              SET_hrdfBIT();
			}
			break;
	}
}

static int memtest3_hack=0;

UINT8 dsp56k_host_interface_read(UINT8 addr)
{
	UINT8 retVal = 0x00;

	switch(addr)
	{
		case 0x00:			// ICR
			retVal = dsp56k.HI.ICR;
			break;

		case 0x01:			// CVR
			retVal = dsp56k.HI.CVR;
			break;

		case 0x02:			// ISR
			retVal = dsp56k.HI.ISR;

			/* We may need some *VERY* tight synchro at the end of the 3rd memory test in order to get this going right */
			if ( (PC <= 0x125) && (PC >= 0x123) )
			{
				/* Maybe this isn't needed? */
				if (!memtest3_hack)
				{
					cpu_boost_interleave(attotime_zero, ATTOTIME_IN_USEC(100));
					memtest3_hack++;
				}
			}
			break;

		case 0x03:			// IVR
			retVal = dsp56k.HI.IVR;
			break;

		case 0x04: break;	// Unused
		case 0x05: break;	// Unused

		case 0x06:			// RXH/TXH
			retVal = dsp56k.HI.TXHRXH;
			break;

		case 0x07:			// RXL/TXL
			retVal = dsp56k.HI.TXLRXL;

			// RXDF is cleared when the Receive Data Low (RXL) register is read by the host processor.
			CLEAR_x_rxdfBIT();
			SET_htdeBIT();			// !!! seems right ???
			break;
	}

	return retVal;
}

/* HACK ! (maybe ;) */
void dsp56k_reset_dma_offset(void)
{
	hack_memory_offset = 0;
}

/**************************************************************************
 * Generic set_info/get_info
 **************************************************************************/

static void dsp56k_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODA:  set_irq_line(DSP56K_IRQ_MODA, info->i); break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODB:  set_irq_line(DSP56K_IRQ_MODB, info->i); break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODC:  set_irq_line(DSP56K_IRQ_MODC, info->i); break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_RESET: set_irq_line(DSP56K_IRQ_RESET, info->i); break;


		// !! It might be interesting to use this section as something which masks out the unecessary bits in each register !!

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + DSP56K_PC:			PC  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_SR:			SR  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_LC:			LC  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_LA:			LA  = info->i & 0xffff;					break;
		case CPUINFO_INT_SP:																	// !!! I think this is correct !!!
		case CPUINFO_INT_REGISTER + DSP56K_SP:			SP  = info->i & 0xff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_OMR:			OMR = info->i & 0xff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_X:			X   = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_Y:			Y   = info->i & 0xffffffff;				break;

		case CPUINFO_INT_REGISTER + DSP56K_A:			A   = info->i & (UINT64)U64(0xffffffffffffffff); break;	// could benefit from a better mask?
		case CPUINFO_INT_REGISTER + DSP56K_B:			B   = info->i & (UINT64)U64(0xffffffffffffffff); break;	// could benefit from a better mask?

		case CPUINFO_INT_REGISTER + DSP56K_R0:			R0  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_R1:			R1  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_R2:			R2  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_R3:			R3  = info->i & 0xffff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_N0:			N0  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_N1:			N1  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_N2:			N2  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_N3:			N3  = info->i & 0xffff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_M0:			M0  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_M1:			M1  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_M2:			M2  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_M3:			M3  = info->i & 0xffff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_TEMP:		TEMP   = info->i & 0xffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_STATUS:		STATUS = info->i & 0xff;				break;

		// The CPU stack...
		case CPUINFO_INT_REGISTER + DSP56K_ST0:			ST0 = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_ST1:			ST1 = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_ST2:			ST2 = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_ST3:			ST3 = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_ST4:			ST4 = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_ST5:			ST5 = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_ST6:			ST6 = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_ST7:			ST7 = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_ST8:			ST8 = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_ST9:			ST9 = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_ST10:		ST10 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST11:		ST11 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST12:		ST12 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST13:		ST13 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST14:		ST14 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST15:		ST15 = info->i & 0xffffffff;			break;
	}
}


void dsp56k_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		// --- the following bits of info are returned as 64-bit signed integers ---
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(dsp56k);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 4;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;	// ?
		case CPUINFO_INT_MAX_CYCLES:					info->i = 8;							break;	// ?

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;	// I think this is the ffc0-fff0 part of data memory?
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;	//
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;	//

		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODA:		info->i = LINE_MODA;				break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODB:		info->i = LINE_MODB;				break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODC:		info->i = LINE_MODC;				break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_RESET:	info->i = LINE_RESET;				break;  // Is reset a special case?

		case CPUINFO_INT_PREVIOUSPC:					info->i = dsp56k.ppc;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + DSP56K_PC:			info->i = PC;							break;
		case CPUINFO_INT_REGISTER + DSP56K_SR:			info->i = SR;							break;
		case CPUINFO_INT_REGISTER + DSP56K_LC:			info->i = LC;							break;
		case CPUINFO_INT_REGISTER + DSP56K_LA:			info->i = LA;							break;
		case CPUINFO_INT_SP:																			// !!! I think this is correct !!!
		case CPUINFO_INT_REGISTER + DSP56K_SP:			info->i = SP;							break;
		case CPUINFO_INT_REGISTER + DSP56K_OMR:			info->i = OMR;							break;

		case CPUINFO_INT_REGISTER + DSP56K_X:			info->i = X;							break;
		case CPUINFO_INT_REGISTER + DSP56K_Y:			info->i = Y;							break;

		case CPUINFO_INT_REGISTER + DSP56K_A:			info->i = A;							break;
		case CPUINFO_INT_REGISTER + DSP56K_B:			info->i = B;							break;

		case CPUINFO_INT_REGISTER + DSP56K_R0:			info->i = R0;							break;
		case CPUINFO_INT_REGISTER + DSP56K_R1:			info->i = R1;							break;
		case CPUINFO_INT_REGISTER + DSP56K_R2:			info->i = R2;							break;
		case CPUINFO_INT_REGISTER + DSP56K_R3:			info->i = R3;							break;

		case CPUINFO_INT_REGISTER + DSP56K_N0:			info->i = N0;							break;
		case CPUINFO_INT_REGISTER + DSP56K_N1:			info->i = N1;							break;
		case CPUINFO_INT_REGISTER + DSP56K_N2:			info->i = N2;							break;
		case CPUINFO_INT_REGISTER + DSP56K_N3:			info->i = N3;							break;

		case CPUINFO_INT_REGISTER + DSP56K_M0:			info->i = M0;							break;
		case CPUINFO_INT_REGISTER + DSP56K_M1:			info->i = M1;							break;
		case CPUINFO_INT_REGISTER + DSP56K_M2:			info->i = M2;							break;
		case CPUINFO_INT_REGISTER + DSP56K_M3:			info->i = M3;							break;

		case CPUINFO_INT_REGISTER + DSP56K_TEMP:		info->i = TEMP;							break;
		case CPUINFO_INT_REGISTER + DSP56K_STATUS:		info->i = STATUS;						break;

		// The CPU stack
		case CPUINFO_INT_REGISTER + DSP56K_ST0:			info->i = ST0;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST1:			info->i = ST1;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST2:			info->i = ST2;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST3:			info->i = ST3;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST4:			info->i = ST4;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST5:			info->i = ST5;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST6:			info->i = ST6;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST7:			info->i = ST7;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST8:			info->i = ST8;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST9:			info->i = ST9;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST10:		info->i = ST10;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST11:		info->i = ST11;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST12:		info->i = ST12;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST13:		info->i = ST13;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST14:		info->i = ST14;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST15:		info->i = ST15;							break;



		// --- the following bits of info are returned as pointers to data or functions ---
		case CPUINFO_PTR_SET_INFO:						info->setinfo = dsp56k_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = dsp56k_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = dsp56k_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = dsp56k_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = dsp56k_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = dsp56k_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = dsp56k_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef ENABLE_DEBUGGER
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = dsp56k_dasm;		break;
#endif /* ENABLE_DEBUGGER */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &dsp56k_icount;			break;
 		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:
 			info->internal_map16 = address_map_dsp56156_x_data_memory;							break;
 		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:
 			info->internal_map16 = address_map_dsp56156_program_memory;							break;
 		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_IO:
 			info->internal_map16 = 0;															break;


		// --- the following bits of info are returned as NULL-terminated strings ---
		case CPUINFO_STR_NAME:							strcpy(info->s, "DSP56156");			break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola DSP56156");	break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.1");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Andrew Gardner");		break;


		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s %s%s %s%s%s%s%s%s%s",
				lfBIT ? "L":".",
				fvBIT ? "F":".",
				s1BIT ? "S":".",
				s0BIT ? "S":".",
				i1BIT ? "I":".",
				i0BIT ? "I":".",
				sBIT ?  "S":".",
				lBIT ?  "L":".",
				eBIT ?  "E":".",
				uBIT ?  "U":".",
				nBIT ?  "N":".",
				zBIT ?  "Z":".",
				vBIT ?  "V":".",
				cBIT ?  "C":".",
				ufBIT ? "U":".",
				seBIT ? "S":".",
				cdBIT ? "C":".",
				sdBIT ? "S":".",
				rBIT ?  "R":".",
				saBIT ? "S":".",
				mcBIT ? "M":".",
				mbBIT ? "M":".",
				maBIT ? "M":".");

            break;


		case CPUINFO_STR_REGISTER + DSP56K_PC:			sprintf(info->s, "PC : %04x", PC);		break;
		case CPUINFO_STR_REGISTER + DSP56K_SR:			sprintf(info->s, "SR : %04x", SR);		break;
		case CPUINFO_STR_REGISTER + DSP56K_LC:			sprintf(info->s, "LC : %04x", LC);		break;
		case CPUINFO_STR_REGISTER + DSP56K_LA:			sprintf(info->s, "LA : %04x", LA);		break;
		case CPUINFO_STR_REGISTER + DSP56K_SP:			sprintf(info->s, "SP : %02x", SP);		break;
		case CPUINFO_STR_REGISTER + DSP56K_OMR:			sprintf(info->s, "OMR: %02x", OMR);		break;

		case CPUINFO_STR_REGISTER + DSP56K_X:			sprintf(info->s, "X  : %04x %04x", X1, X0); break;
		case CPUINFO_STR_REGISTER + DSP56K_Y:			sprintf(info->s, "Y  : %04x %04x", Y1, Y0); break;

		// !! This is silly - it gives me a warning if I try to print an unsigned long with %08x
		//    (and thus won't compile) - any suggestions?  Maybe we change it to a series of UINT16's or something?
		case CPUINFO_STR_REGISTER + DSP56K_A:			sprintf(info->s, "A  : %02x %04x %04x", A2,A1,A0); break;
		case CPUINFO_STR_REGISTER + DSP56K_B:			sprintf(info->s, "B  : %02x %04x %04x", B2,B1,B0); break;

		case CPUINFO_STR_REGISTER + DSP56K_R0:			sprintf(info->s, "R0 : %04x", R0);		break;
		case CPUINFO_STR_REGISTER + DSP56K_R1:			sprintf(info->s, "R1 : %04x", R1);		break;
		case CPUINFO_STR_REGISTER + DSP56K_R2:			sprintf(info->s, "R2 : %04x", R2);		break;
		case CPUINFO_STR_REGISTER + DSP56K_R3:			sprintf(info->s, "R3 : %04x", R3);		break;

		case CPUINFO_STR_REGISTER + DSP56K_N0:			sprintf(info->s, "N0 : %04x", N0);		break;
		case CPUINFO_STR_REGISTER + DSP56K_N1:			sprintf(info->s, "N1 : %04x", N1);		break;
		case CPUINFO_STR_REGISTER + DSP56K_N2:			sprintf(info->s, "N2 : %04x", N2);		break;
		case CPUINFO_STR_REGISTER + DSP56K_N3:			sprintf(info->s, "N3 : %04x", N3);		break;

		case CPUINFO_STR_REGISTER + DSP56K_M0:			sprintf(info->s, "M0 : %04x", M0);		break;
		case CPUINFO_STR_REGISTER + DSP56K_M1:			sprintf(info->s, "M1 : %04x", M1);		break;
		case CPUINFO_STR_REGISTER + DSP56K_M2:			sprintf(info->s, "M2 : %04x", M2);		break;
		case CPUINFO_STR_REGISTER + DSP56K_M3:			sprintf(info->s, "M3 : %04x", M3);		break;

		case CPUINFO_STR_REGISTER + DSP56K_TEMP:		sprintf(info->s, "TMP: %04x", TEMP);	break;
		case CPUINFO_STR_REGISTER + DSP56K_STATUS:		sprintf(info->s, "STS: %02x", STATUS);	break;


		// The CPU stack
		case CPUINFO_STR_REGISTER + DSP56K_ST0:			sprintf(info->s, "ST0 : %08x", ST0);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST1:			sprintf(info->s, "ST1 : %08x", ST1);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST2:			sprintf(info->s, "ST2 : %08x", ST2);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST3:			sprintf(info->s, "ST3 : %08x", ST3);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST4:			sprintf(info->s, "ST4 : %08x", ST4);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST5:			sprintf(info->s, "ST5 : %08x", ST5);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST6:			sprintf(info->s, "ST6 : %08x", ST6);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST7:			sprintf(info->s, "ST7 : %08x", ST7);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST8:			sprintf(info->s, "ST8 : %08x", ST8);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST9:			sprintf(info->s, "ST9 : %08x", ST9);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST10:		sprintf(info->s, "ST10: %08x", ST10);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST11:		sprintf(info->s, "ST11: %08x", ST11);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST12:		sprintf(info->s, "ST12: %08x", ST12);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST13:		sprintf(info->s, "ST13: %08x", ST13);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST14:		sprintf(info->s, "ST14: %08x", ST14);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST15:		sprintf(info->s, "ST15: %08x", ST15);	break;
	}
}
