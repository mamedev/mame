// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef __DSP56_MEM_H__
#define __DSP56_MEM_H__

#include "dsp56k.h"

namespace DSP56K
{
/***************************************************************************
    MEMORY
***************************************************************************/
void mem_reset(dsp56k_core* cpustate);

// Adjusts the documented address to match the offset in peripheral RAM
#define A2O(a) (a - 0xffc0)

// Adjusts the offset in peripheral RAM to match the documented address
#define O2A(a) (a + 0xffc0)

// The memory 'registers'
#define PBC      (cpustate->peripheral_ram[A2O(0xffc0)])
#define PCC      (cpustate->peripheral_ram[A2O(0xffc1)])
#define PBDDR    (cpustate->peripheral_ram[A2O(0xffc2)])
#define PCDDR    (cpustate->peripheral_ram[A2O(0xffc3)])
#define HCR      (cpustate->peripheral_ram[A2O(0xffc4)])
#define COCR     (cpustate->peripheral_ram[A2O(0xffc8)])
#define CRASSI0  (cpustate->peripheral_ram[A2O(0xffd0)])
#define CRBSSI0  (cpustate->peripheral_ram[A2O(0xffd1)])
#define CRASSI1  (cpustate->peripheral_ram[A2O(0xffd8)])
#define CRBSSI1  (cpustate->peripheral_ram[A2O(0xffd9)])
#define PLCR     (cpustate->peripheral_ram[A2O(0xffdc)])
#define BCR      (cpustate->peripheral_ram[A2O(0xffde)])
#define IPR      (cpustate->peripheral_ram[A2O(0xffdf)])
#define PBD      (cpustate->peripheral_ram[A2O(0xffe2)])
#define PCD      (cpustate->peripheral_ram[A2O(0xffe3)])
#define HSR      (cpustate->peripheral_ram[A2O(0xffe4)])
#define HTXHRX   (cpustate->peripheral_ram[A2O(0xffe5)])
#define COSR     (cpustate->peripheral_ram[A2O(0xffe8)])
#define CRXCTX   (cpustate->peripheral_ram[A2O(0xffe9)])
#define TCR      (cpustate->peripheral_ram[A2O(0xffec)])
#define TCTR     (cpustate->peripheral_ram[A2O(0xffed)])
#define TCPR     (cpustate->peripheral_ram[A2O(0xffee)])
#define TPR      (cpustate->peripheral_ram[A2O(0xffef)])
#define TSRSSI0  (cpustate->peripheral_ram[A2O(0xfff0)])
#define TRXSSI0  (cpustate->peripheral_ram[A2O(0xfff1)])
#define RSMA0    (cpustate->peripheral_ram[A2O(0xfff2)])
#define RSMB0    (cpustate->peripheral_ram[A2O(0xfff3)])
#define TSMA0    (cpustate->peripheral_ram[A2O(0xfff4)])
#define TSMB0    (cpustate->peripheral_ram[A2O(0xfff5)])
#define TSRSSI1  (cpustate->peripheral_ram[A2O(0xfff8)])
#define TRXSSI1  (cpustate->peripheral_ram[A2O(0xfff9)])
#define RSMA1    (cpustate->peripheral_ram[A2O(0xfffa)])
#define RSMB1    (cpustate->peripheral_ram[A2O(0xfffb)])
#define TSMA1    (cpustate->peripheral_ram[A2O(0xfffc)])
#define TSMB1    (cpustate->peripheral_ram[A2O(0xfffd)])

/* Interrupt priority register (IPR) bits */
void IPR_set(dsp56k_core* cpustate, UINT16 value);

/* A return value of -1 means disabled */
INT8 irqa_ipl(dsp56k_core* cpustate);
INT8 irqb_ipl(dsp56k_core* cpustate);
UINT8 irqa_trigger(dsp56k_core* cpustate);
UINT8 irqb_trigger(dsp56k_core* cpustate);

INT8 codec_ipl(dsp56k_core* cpustate);
INT8 host_ipl(dsp56k_core* cpustate);
INT8 ssi0_ipl(dsp56k_core* cpustate);
INT8 ssi1_ipl(dsp56k_core* cpustate);
INT8 tm_ipl(dsp56k_core* cpustate);


/***************************************************************************
    HOST INTERFACE
***************************************************************************/
void dsp56k_host_interface_reset(dsp56k_core* cpustate);
#define HTX (HTXHRX)
#define HRX (HTXHRX)

#define ICR (cpustate->HI.icr)
#define CVR (cpustate->HI.cvr)
#define ISR (cpustate->HI.isr)
#define IVR (cpustate->HI.ivr)
#define TXH (cpustate->HI.trxh)
#define TXL (cpustate->HI.trxl)
#define RXH (cpustate->HI.trxh)
#define RXL (cpustate->HI.trxl)

/***************/
/* DSP56k SIDE */
/***************/
/* Host Control Register (HCR) Bits */
void HCR_set(dsp56k_core* cpustate, UINT16 value);

//UINT16 HF3_bit(dsp56k_core* cpustate);  #define hf3BIT  ((HCR & 0x0010) != 0)
//UINT16 HF2_bit(dsp56k_core* cpustate);  #define hf2BIT  ((HCR & 0x0008) != 0)
UINT16 HCIE_bit(dsp56k_core* cpustate);
UINT16 HTIE_bit(dsp56k_core* cpustate);
UINT16 HRIE_bit(dsp56k_core* cpustate);

void HF3_bit_set(dsp56k_core* cpustate, UINT16 value);
void HF2_bit_set(dsp56k_core* cpustate, UINT16 value);
void HCIE_bit_set(dsp56k_core* cpustate, UINT16 value);
void HTIE_bit_set(dsp56k_core* cpustate, UINT16 value);
void HRIE_bit_set(dsp56k_core* cpustate, UINT16 value);

/* Host Status Register (HSR) Bits */
//void HSR_set(dsp56k_core* cpustate, UINT16 value);

//UINT16 DMA_bit(dsp56k_core* cpustate);  #define dmaBIT  ((HSR & 0x0080) != 0)
//UINT16 HF1_bit(dsp56k_core* cpustate);  #define hf1BIT  ((HSR & 0x0010) != 0)
//UINT16 HF0_bit(dsp56k_core* cpustate);  #define hf0BIT  ((HSR & 0x0008) != 0)
//UINT16 HCP_bit(dsp56k_core* cpustate);  #define hcpBIT  ((HSR & 0x0004) != 0)
UINT16 HTDE_bit(dsp56k_core* cpustate);
UINT16 HRDF_bit(dsp56k_core* cpustate);

void DMA_bit_set(dsp56k_core* cpustate, UINT16 value);
void HF1_bit_set(dsp56k_core* cpustate, UINT16 value);
void HF0_bit_set(dsp56k_core* cpustate, UINT16 value);
void HCP_bit_set(dsp56k_core* cpustate, UINT16 value);
void HTDE_bit_set(dsp56k_core* cpustate, UINT16 value);
void HRDF_bit_set(dsp56k_core* cpustate, UINT16 value);

/*************/
/* HOST SIDE */
/*************/
/* Interrupt Control Register (ICR) Bits */
void ICR_set(dsp56k_core* cpustate, UINT8 value);

//UINT8 INIT_bit(dsp56k_core* cpustate); #define x_initBIT ((dsp56k.HI.ICR & 0x0080) != 0)
//UINT8 HM1_bit(dsp56k_core* cpustate);  #define x_hm1BIT  ((dsp56k.HI.ICR & 0x0040) != 0)
//UINT8 HM0_bit(dsp56k_core* cpustate);  #define x_hm0BIT  ((dsp56k.HI.ICR & 0x0020) != 0)
//UINT8 HF1_bit_host(dsp56k_core* cpustate);  #define x_hf1BIT  ((dsp56k.HI.ICR & 0x0010) != 0)
//UINT8 HF0_bit_host(dsp56k_core* cpustate);  #define x_hf0BIT  ((dsp56k.HI.ICR & 0x0008) != 0)
//UINT8 TREQ_bit(dsp56k_core* cpustate); #define x_treqBIT ((dsp56k.HI.ICR & 0x0002) != 0)
//UINT8 RREQ_bit(dsp56k_core* cpustate); #define x_rreqBIT ((dsp56k.HI.ICR & 0x0001) != 0)

//void INIT_bit_set(dsp56k_core* cpustate, UINT8 value); #define CLEAR_x_initBIT() (dsp56k.HI.ICR &= (~0x0080))
//void HM1_bit_set(dsp56k_core* cpustate, UINT8 value);  #define CLEAR_x_hm1BIT()  (dsp56k.HI.ICR &= (~0x0040))
//void HM0_bit_set(dsp56k_core* cpustate, UINT8 value);  #define CLEAR_x_hm0BIT()  (dsp56k.HI.ICR &= (~0x0020))
void HF1_bit_host_set(dsp56k_core* cpustate, UINT8 value);
void HF0_bit_host_set(dsp56k_core* cpustate, UINT8 value);
void TREQ_bit_set(dsp56k_core* cpustate, UINT8 value);
void RREQ_bit_set(dsp56k_core* cpustate, UINT8 value);

/* Command Vector Register (CVR) Bits */
void CVR_set(dsp56k_core* cpustate, UINT8 value);

//UINT8 HC_bit();
UINT8 HV_bits(dsp56k_core* cpustate);

void HC_bit_set(dsp56k_core* cpustate, UINT8 value);
void HV_bits_set(dsp56k_core* cpustate, UINT8 value);

/* Interrupt Status Register (ISR) Bits */
// void ISR_set(dsp56k_core* cpustate, UINT8 value);

//UINT8 HREQ_bit(dsp56k_core* cpustate); #define x_hreqBIT ((dsp56k.HI.ISR & 0x0080) != 0)
//UINT8 DMA_bit(dsp56k_core* cpustate);  #define x_dmaBIT  ((dsp56k.HI.ISR & 0x0040) != 0)
//UINT8 HF3_bit_host(dsp56k_core* cpustate);  #define x_hf3BIT  ((dsp56k.HI.ISR & 0x0010) != 0)
//UINT8 HF2_bit_host(dsp56k_core* cpustate);  #define x_hf2BIT  ((dsp56k.HI.ISR & 0x0008) != 0)
//UINT8 TRDY_bit(dsp56k_core* cpustate); #define x_trdyBIT ((dsp56k.HI.ISR & 0x0004) != 0)
UINT8 TXDE_bit(dsp56k_core* cpustate);
UINT8 RXDF_bit(dsp56k_core* cpustate);

//void HREQ_bit_set(dsp56k_core* cpustate, UINT8 value); #define CLEAR_x_hreqBIT() (dsp56k.HI.ISR &= (~0x0080))
//void DMA_bit_set(dsp56k_core* cpustate, UINT8 value);  #define CLEAR_x_dmaBIT()  (dsp56k.HI.ISR &= (~0x0040))
void HF3_bit_host_set(dsp56k_core* cpustate, UINT8 value);
void HF2_bit_host_set(dsp56k_core* cpustate, UINT8 value);
//void TRDY_bit_set(dsp56k_core* cpustate, UINT8 value); #define CLEAR_x_trdyBIT() (dsp56k.HI.ISR &= (~0x0004))
void TXDE_bit_set(dsp56k_core* cpustate, UINT8 value);
void RXDF_bit_set(dsp56k_core* cpustate, UINT8 value);

/* Interrupt Vector Register (IVR) Bits */
//void IVR_set(dsp56k_core* cpustate, UINT8 value);

//UINT8 IV7_bit(dsp56k_core* cpustate);
//UINT8 IV6_bit(dsp56k_core* cpustate);
//UINT8 IV5_bit(dsp56k_core* cpustate);
//UINT8 IV4_bit(dsp56k_core* cpustate);
//UINT8 IV3_bit(dsp56k_core* cpustate);
//UINT8 IV2_bit(dsp56k_core* cpustate);
//UINT8 IV1_bit(dsp56k_core* cpustate);
//UINT8 IV0_bit(dsp56k_core* cpustate);

//void IV7_bit_set(dsp56k_core* cpustate, UINT8 value);
//void IV6_bit_set(dsp56k_core* cpustate, UINT8 value);
//void IV5_bit_set(dsp56k_core* cpustate, UINT8 value);
//void IV4_bit_set(dsp56k_core* cpustate, UINT8 value);
//void IV3_bit_set(dsp56k_core* cpustate, UINT8 value);
//void IV2_bit_set(dsp56k_core* cpustate, UINT8 value);
//void IV1_bit_set(dsp56k_core* cpustate, UINT8 value);
//void IV0_bit_set(dsp56k_core* cpustate, UINT8 value);


/* PROTOTYPES */
void dsp56k_host_interface_HTX_to_host(dsp56k_core* cpustate);
void dsp56k_host_interface_host_to_HTX(dsp56k_core* cpustate);


/***************************************************************************
    I/O INTERFACE
***************************************************************************/
void dsp56k_io_reset(dsp56k_core* cpustate);

/* Port A Bus Control Register (BCR) */
void BCR_set(dsp56k_core* cpustate, UINT16 value);

//UINT16 RH_bit(dsp56k_core* cpustate);
//UINT16 BS_bit(dsp56k_core* cpustate);
//UINT16 external_x_wait_states(dsp56k_core* cpustate);
//UINT16 external_p_wait_states(dsp56k_core* cpustate);

void RH_bit_set(dsp56k_core* cpustate, UINT16 value);
void BS_bit_set(dsp56k_core* cpustate, UINT16 value);
void external_x_wait_states_set(dsp56k_core* cpustate, UINT16 value);
void external_p_wait_states_set(dsp56k_core* cpustate, UINT16 value);

/* Port B Control Register (PBC) */
void PBC_set(dsp56k_core* cpustate, UINT16 value);
//int host_interface_active(dsp56k_core* cpustate);

/* Port B Data Direction Register (PBDDR) */
void PBDDR_set(dsp56k_core* cpustate, UINT16 value);

/* Port B Data Register (PBD) */
void PBD_set(dsp56k_core* cpustate, UINT16 value);

/* Port C Control Register (PCC) */
void PCC_set(dsp56k_core* cpustate, UINT16 value);

/* Port C Data Direction Register (PCDDR) */
void PCDDR_set(dsp56k_core* cpustate, UINT16 value);

/* Port C Dtaa Register (PCD) */
void PCD_set(dsp56k_core* cpustate, UINT16 value);

} // namespace DSP56K

#endif
