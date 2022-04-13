// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef MAME_CPU_DSP56156_DSP56MEM_H
#define MAME_CPU_DSP56156_DSP56MEM_H

#include "dsp56156.h"

namespace DSP_56156 {

/***************************************************************************
    MEMORY
***************************************************************************/
void mem_reset(dsp56156_core* cpustate);

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
void IPR_set(dsp56156_core* cpustate, uint16_t value);

/* A return value of -1 means disabled */
int8_t irqa_ipl(dsp56156_core* cpustate);
int8_t irqb_ipl(dsp56156_core* cpustate);
uint8_t irqa_trigger(dsp56156_core* cpustate);
uint8_t irqb_trigger(dsp56156_core* cpustate);

int8_t codec_ipl(dsp56156_core* cpustate);
int8_t host_ipl(dsp56156_core* cpustate);
int8_t ssi0_ipl(dsp56156_core* cpustate);
int8_t ssi1_ipl(dsp56156_core* cpustate);
int8_t tm_ipl(dsp56156_core* cpustate);


/***************************************************************************
    HOST INTERFACE
***************************************************************************/
void dsp56156_host_interface_reset(dsp56156_core* cpustate);
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

/*****************/
/* DSP56156 SIDE */
/*****************/
/* Host Control Register (HCR) Bits */
void HCR_set(dsp56156_core* cpustate, uint16_t value);

//uint16_t HF3_bit(dsp56156_core* cpustate);  #define hf3BIT  ((HCR & 0x0010) != 0)
//uint16_t HF2_bit(dsp56156_core* cpustate);  #define hf2BIT  ((HCR & 0x0008) != 0)
uint16_t HCIE_bit(dsp56156_core* cpustate);
uint16_t HTIE_bit(dsp56156_core* cpustate);
uint16_t HRIE_bit(dsp56156_core* cpustate);

void HF3_bit_set(dsp56156_core* cpustate, uint16_t value);
void HF2_bit_set(dsp56156_core* cpustate, uint16_t value);
void HCIE_bit_set(dsp56156_core* cpustate, uint16_t value);
void HTIE_bit_set(dsp56156_core* cpustate, uint16_t value);
void HRIE_bit_set(dsp56156_core* cpustate, uint16_t value);

/* Host Status Register (HSR) Bits */
//void HSR_set(dsp56156_core* cpustate, uint16_t value);

//uint16_t DMA_bit(dsp56156_core* cpustate);  #define dmaBIT  ((HSR & 0x0080) != 0)
//uint16_t HF1_bit(dsp56156_core* cpustate);  #define hf1BIT  ((HSR & 0x0010) != 0)
//uint16_t HF0_bit(dsp56156_core* cpustate);  #define hf0BIT  ((HSR & 0x0008) != 0)
//uint16_t HCP_bit(dsp56156_core* cpustate);  #define hcpBIT  ((HSR & 0x0004) != 0)
uint16_t HTDE_bit(dsp56156_core* cpustate);
uint16_t HRDF_bit(dsp56156_core* cpustate);

void DMA_bit_set(dsp56156_core* cpustate, uint16_t value);
void HF1_bit_set(dsp56156_core* cpustate, uint16_t value);
void HF0_bit_set(dsp56156_core* cpustate, uint16_t value);
void HCP_bit_set(dsp56156_core* cpustate, uint16_t value);
void HTDE_bit_set(dsp56156_core* cpustate, uint16_t value);
void HRDF_bit_set(dsp56156_core* cpustate, uint16_t value);

/*************/
/* HOST SIDE */
/*************/
/* Interrupt Control Register (ICR) Bits */
void ICR_set(dsp56156_core* cpustate, uint8_t value);

//uint8_t INIT_bit(dsp56156_core* cpustate); #define x_initBIT ((dsp56156.HI.ICR & 0x0080) != 0)
//uint8_t HM1_bit(dsp56156_core* cpustate);  #define x_hm1BIT  ((dsp56156.HI.ICR & 0x0040) != 0)
//uint8_t HM0_bit(dsp56156_core* cpustate);  #define x_hm0BIT  ((dsp56156.HI.ICR & 0x0020) != 0)
//uint8_t HF1_bit_host(dsp56156_core* cpustate);  #define x_hf1BIT  ((dsp56156.HI.ICR & 0x0010) != 0)
//uint8_t HF0_bit_host(dsp56156_core* cpustate);  #define x_hf0BIT  ((dsp56156.HI.ICR & 0x0008) != 0)
//uint8_t TREQ_bit(dsp56156_core* cpustate); #define x_treqBIT ((dsp56156.HI.ICR & 0x0002) != 0)
//uint8_t RREQ_bit(dsp56156_core* cpustate); #define x_rreqBIT ((dsp56156.HI.ICR & 0x0001) != 0)

//void INIT_bit_set(dsp56156_core* cpustate, uint8_t value); #define CLEAR_x_initBIT() (dsp56156.HI.ICR &= (~0x0080))
//void HM1_bit_set(dsp56156_core* cpustate, uint8_t value);  #define CLEAR_x_hm1BIT()  (dsp56156.HI.ICR &= (~0x0040))
//void HM0_bit_set(dsp56156_core* cpustate, uint8_t value);  #define CLEAR_x_hm0BIT()  (dsp56156.HI.ICR &= (~0x0020))
void HF1_bit_host_set(dsp56156_core* cpustate, uint8_t value);
void HF0_bit_host_set(dsp56156_core* cpustate, uint8_t value);
void TREQ_bit_set(dsp56156_core* cpustate, uint8_t value);
void RREQ_bit_set(dsp56156_core* cpustate, uint8_t value);

/* Command Vector Register (CVR) Bits */
void CVR_set(dsp56156_core* cpustate, uint8_t value);

//uint8_t HC_bit();
uint8_t HV_bits(dsp56156_core* cpustate);

void HC_bit_set(dsp56156_core* cpustate, uint8_t value);
void HV_bits_set(dsp56156_core* cpustate, uint8_t value);

/* Interrupt Status Register (ISR) Bits */
// void ISR_set(dsp56156_core* cpustate, uint8_t value);

//uint8_t HREQ_bit(dsp56156_core* cpustate); #define x_hreqBIT ((dsp56156.HI.ISR & 0x0080) != 0)
//uint8_t DMA_bit(dsp56156_core* cpustate);  #define x_dmaBIT  ((dsp56156.HI.ISR & 0x0040) != 0)
//uint8_t HF3_bit_host(dsp56156_core* cpustate);  #define x_hf3BIT  ((dsp56156.HI.ISR & 0x0010) != 0)
//uint8_t HF2_bit_host(dsp56156_core* cpustate);  #define x_hf2BIT  ((dsp56156.HI.ISR & 0x0008) != 0)
//uint8_t TRDY_bit(dsp56156_core* cpustate); #define x_trdyBIT ((dsp56156.HI.ISR & 0x0004) != 0)
uint8_t TXDE_bit(dsp56156_core* cpustate);
uint8_t RXDF_bit(dsp56156_core* cpustate);

//void HREQ_bit_set(dsp56156_core* cpustate, uint8_t value); #define CLEAR_x_hreqBIT() (dsp56156.HI.ISR &= (~0x0080))
//void DMA_bit_set(dsp56156_core* cpustate, uint8_t value);  #define CLEAR_x_dmaBIT()  (dsp56156.HI.ISR &= (~0x0040))
void HF3_bit_host_set(dsp56156_core* cpustate, uint8_t value);
void HF2_bit_host_set(dsp56156_core* cpustate, uint8_t value);
//void TRDY_bit_set(dsp56156_core* cpustate, uint8_t value); #define CLEAR_x_trdyBIT() (dsp56156.HI.ISR &= (~0x0004))
void TXDE_bit_set(dsp56156_core* cpustate, uint8_t value);
void RXDF_bit_set(dsp56156_core* cpustate, uint8_t value);

/* Interrupt Vector Register (IVR) Bits */
//void IVR_set(dsp56156_core* cpustate, uint8_t value);

//uint8_t IV7_bit(dsp56156_core* cpustate);
//uint8_t IV6_bit(dsp56156_core* cpustate);
//uint8_t IV5_bit(dsp56156_core* cpustate);
//uint8_t IV4_bit(dsp56156_core* cpustate);
//uint8_t IV3_bit(dsp56156_core* cpustate);
//uint8_t IV2_bit(dsp56156_core* cpustate);
//uint8_t IV1_bit(dsp56156_core* cpustate);
//uint8_t IV0_bit(dsp56156_core* cpustate);

//void IV7_bit_set(dsp56156_core* cpustate, uint8_t value);
//void IV6_bit_set(dsp56156_core* cpustate, uint8_t value);
//void IV5_bit_set(dsp56156_core* cpustate, uint8_t value);
//void IV4_bit_set(dsp56156_core* cpustate, uint8_t value);
//void IV3_bit_set(dsp56156_core* cpustate, uint8_t value);
//void IV2_bit_set(dsp56156_core* cpustate, uint8_t value);
//void IV1_bit_set(dsp56156_core* cpustate, uint8_t value);
//void IV0_bit_set(dsp56156_core* cpustate, uint8_t value);


/* PROTOTYPES */
void dsp56156_host_interface_HTX_to_host(dsp56156_core* cpustate);
void dsp56156_host_interface_host_to_HTX(dsp56156_core* cpustate);


/***************************************************************************
    I/O INTERFACE
***************************************************************************/
void dsp56156_io_reset(dsp56156_core* cpustate);

/* Port A Bus Control Register (BCR) */
void BCR_set(dsp56156_core* cpustate, uint16_t value);

//uint16_t RH_bit(dsp56156_core* cpustate);
//uint16_t BS_bit(dsp56156_core* cpustate);
//uint16_t external_x_wait_states(dsp56156_core* cpustate);
//uint16_t external_p_wait_states(dsp56156_core* cpustate);

void RH_bit_set(dsp56156_core* cpustate, uint16_t value);
void BS_bit_set(dsp56156_core* cpustate, uint16_t value);
void external_x_wait_states_set(dsp56156_core* cpustate, uint16_t value);
void external_p_wait_states_set(dsp56156_core* cpustate, uint16_t value);

/* Port B Control Register (PBC) */
void PBC_set(dsp56156_core* cpustate, uint16_t value);
//int host_interface_active(dsp56156_core* cpustate);

/* Port B Data Direction Register (PBDDR) */
void PBDDR_set(dsp56156_core* cpustate, uint16_t value);

/* Port B Data Register (PBD) */
void PBD_set(dsp56156_core* cpustate, uint16_t value);

/* Port C Control Register (PCC) */
void PCC_set(dsp56156_core* cpustate, uint16_t value);

/* Port C Data Direction Register (PCDDR) */
void PCDDR_set(dsp56156_core* cpustate, uint16_t value);

/* Port C Dtaa Register (PCD) */
void PCD_set(dsp56156_core* cpustate, uint16_t value);

} // namespace DSP_56156

#endif // MAME_CPU_DSP56156_DSP56MEM_H
