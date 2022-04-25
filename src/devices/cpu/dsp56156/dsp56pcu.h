// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef DSP56156_PCU_H
#define DSP56156_PCU_H

#include "dsp56156.h"

namespace DSP_56156
{
/***************************************************************************
    PCU
***************************************************************************/
void pcu_reset(dsp56156_core* cpustate);
void pcu_init(dsp56156_core* cpustate, device_t *device);
#define PC  (cpustate->PCU.pc)
#define LA  (cpustate->PCU.la)
#define LC  (cpustate->PCU.lc)
#define SR  (cpustate->PCU.sr)
#define OMR (cpustate->PCU.omr)
#define SP  (cpustate->PCU.sp)
#define SS  (cpustate->PCU.ss)

#define SSH (SS[SP].w.h)
#define SSL (SS[SP].w.l)

#define ST0     (SS[0].d)
#define ST1     (SS[1].d)
#define ST2     (SS[2].d)
#define ST3     (SS[3].d)
#define ST4     (SS[4].d)
#define ST5     (SS[5].d)
#define ST6     (SS[6].d)
#define ST7     (SS[7].d)
#define ST8     (SS[8].d)
#define ST9     (SS[9].d)
#define ST10    (SS[10].d)
#define ST11    (SS[11].d)
#define ST12    (SS[12].d)
#define ST13    (SS[13].d)
#define ST14    (SS[14].d)
#define ST15    (SS[15].d)

/* STATUS REGISTER (SR) BITS (1-25) */
/* MR */
uint8_t LF_bit(const dsp56156_core* cpustate);
uint8_t FV_bit(const dsp56156_core* cpustate);
//uint8_t S_bits(const dsp56156_core* cpustate);
uint8_t I_bits(const dsp56156_core* cpustate);

/* CCR - with macros for easy access */
#define S() (S_bit(cpustate))
uint8_t S_bit(const dsp56156_core* cpustate);
#define L() (L_bit(cpustate))
uint8_t L_bit(const dsp56156_core* cpustate);
#define E() (E_bit(cpustate))
uint8_t E_bit(const dsp56156_core* cpustate);
#define U() (U_bit(cpustate))
uint8_t U_bit(const dsp56156_core* cpustate);
#define N() (N_bit(cpustate))
uint8_t N_bit(const dsp56156_core* cpustate);
#define Z() (Z_bit(cpustate))
uint8_t Z_bit(const dsp56156_core* cpustate);
#define V() (V_bit(cpustate))
uint8_t V_bit(const dsp56156_core* cpustate);
#define C() (C_bit(cpustate))
uint8_t C_bit(const dsp56156_core* cpustate);

/* MR setters */
void LF_bit_set(dsp56156_core* cpustate, uint8_t value);
void FV_bit_set(dsp56156_core* cpustate, uint8_t value);
void S_bits_set(dsp56156_core* cpustate, uint8_t value);
void I_bits_set(dsp56156_core* cpustate, uint8_t value);

/* CCR setters - with macros for easy access */
#define DSP56156_S_SET() (S_bit_set(cpustate, 1))
#define DSP56156_S_CLEAR() (S_bit_set(cpustate, 0))
void S_bit_set(dsp56156_core* cpustate, uint8_t value);
#define DSP56156_L_SET() (L_bit_set(cpustate, 1))
#define DSP56156_L_CLEAR() (L_bit_set(cpustate, 0))
void L_bit_set(dsp56156_core* cpustate, uint8_t value);
#define DSP56156_E_SET() (E_bit_set(cpustate, 1))
#define DSP56156_E_CLEAR() (E_bit_set(cpustate, 0))
void E_bit_set(dsp56156_core* cpustate, uint8_t value);
#define DSP56156_U_SET() (U_bit_set(cpustate, 1))
#define DSP56156_U_CLEAR() (U_bit_set(cpustate, 0))
void U_bit_set(dsp56156_core* cpustate, uint8_t value);
#define DSP56156_N_SET() (N_bit_set(cpustate, 1))
#define DSP56156_N_CLEAR() (N_bit_set(cpustate, 0))
void N_bit_set(dsp56156_core* cpustate, uint8_t value);
#define DSP56156_Z_SET() (Z_bit_set(cpustate, 1))
#define DSP56156_Z_CLEAR() (Z_bit_set(cpustate, 0))
void Z_bit_set(dsp56156_core* cpustate, uint8_t value);
#define DSP56156_V_SET() (V_bit_set(cpustate, 1))
#define DSP56156_V_CLEAR() (V_bit_set(cpustate, 0))
void V_bit_set(dsp56156_core* cpustate, uint8_t value);
#define DSP56156_C_SET() (C_bit_set(cpustate, 1))
#define DSP56156_C_CLEAR() (C_bit_set(cpustate, 0))
void C_bit_set(dsp56156_core* cpustate, uint8_t value);

// TODO: Maybe some functions for Interrupt Mask and Scaling Mode go here?


/* 1-28 OPERATING MODE REGISTER (OMR) BITS */
//uint8_t CD_bit(const dsp56156_core* cpustate);
//uint8_t SD_bit(const dsp56156_core* cpustate);
//uint8_t R_bit(const dsp56156_core* cpustate);
//uint8_t SA_bit(const dsp56156_core* cpustate);
//uint8_t MC_bit(const dsp56156_core* cpustate);
uint8_t MB_bit(const dsp56156_core* cpustate);
uint8_t MA_bit(const dsp56156_core* cpustate);

void CD_bit_set(dsp56156_core* cpustate, uint8_t value);
void SD_bit_set(dsp56156_core* cpustate, uint8_t value);
void R_bit_set(dsp56156_core* cpustate, uint8_t value);
void SA_bit_set(dsp56156_core* cpustate, uint8_t value);
void MC_bit_set(dsp56156_core* cpustate, uint8_t value);
void MB_bit_set(dsp56156_core* cpustate, uint8_t value);
void MA_bit_set(dsp56156_core* cpustate, uint8_t value);

/* 1-27 STACK POINTER (SP) BITS */
uint8_t UF_bit(const dsp56156_core* cpustate);
uint8_t SE_bit(const dsp56156_core* cpustate);

//void UF_bit_set(dsp56156_core* cpustate, uint8_t value) {}
//void SE_bit_set(dsp56156_core* cpustate, uint8_t value) {}


// HACK - Bootstrap modes
#define BOOTSTRAP_OFF (0)
#define BOOTSTRAP_SSIX (1)
#define BOOTSTRAP_HI (2)


/* PCU IRQ goodies */
void pcu_service_interrupts(dsp56156_core* cpustate);

void dsp56156_irq_table_init(void);
void dsp56156_set_irq_source(uint8_t irq_num, uint16_t iv, const char* source);
int dsp56156_get_irq_index_by_tag(const char* tag);

void dsp56156_add_pending_interrupt(dsp56156_core* cpustate, const char* name);     // Call me to add an interrupt to the queue

void dsp56156_clear_pending_interrupts(dsp56156_core* cpustate);
int dsp56156_count_pending_interrupts(dsp56156_core* cpustate);
void dsp56156_sort_pending_interrupts(dsp56156_core* cpustate, int num);
int8_t dsp56156_get_irq_priority(dsp56156_core* cpustate, int index);

} // namespace DSP_56156

#endif
