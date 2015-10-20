// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#ifndef __DSP56_PCU_H__
#define __DSP56_PCU_H__

#include "dsp56k.h"

namespace DSP56K
{
/***************************************************************************
    PCU
***************************************************************************/
void pcu_reset(dsp56k_core* cpustate);
void pcu_init(dsp56k_core* cpustate, device_t *device);
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
UINT8 LF_bit(dsp56k_core* cpustate);
UINT8 FV_bit(dsp56k_core* cpustate);
//UINT8 S_bits(dsp56k_core* cpustate);
UINT8 I_bits(dsp56k_core* cpustate);

/* CCR - with macros for easy access */
#define S() (S_bit(cpustate))
UINT8 S_bit(dsp56k_core* cpustate);
#define L() (L_bit(cpustate))
UINT8 L_bit(dsp56k_core* cpustate);
#define E() (E_bit(cpustate))
UINT8 E_bit(dsp56k_core* cpustate);
#define U() (U_bit(cpustate))
UINT8 U_bit(dsp56k_core* cpustate);
#define N() (N_bit(cpustate))
UINT8 N_bit(dsp56k_core* cpustate);
#define Z() (Z_bit(cpustate))
UINT8 Z_bit(dsp56k_core* cpustate);
#define V() (V_bit(cpustate))
UINT8 V_bit(dsp56k_core* cpustate);
#define C() (C_bit(cpustate))
UINT8 C_bit(dsp56k_core* cpustate);

/* MR setters */
void LF_bit_set(dsp56k_core* cpustate, UINT8 value);
void FV_bit_set(dsp56k_core* cpustate, UINT8 value);
void S_bits_set(dsp56k_core* cpustate, UINT8 value);
void I_bits_set(dsp56k_core* cpustate, UINT8 value);

/* CCR setters - with macros for easy access */
#define DSP56K_S_SET() (S_bit_set(cpustate, 1))
#define DSP56K_S_CLEAR() (S_bit_set(cpustate, 0))
void S_bit_set(dsp56k_core* cpustate, UINT8 value);
#define DSP56K_L_SET() (L_bit_set(cpustate, 1))
#define DSP56K_L_CLEAR() (L_bit_set(cpustate, 0))
void L_bit_set(dsp56k_core* cpustate, UINT8 value);
#define DSP56K_E_SET() (E_bit_set(cpustate, 1))
#define DSP56K_E_CLEAR() (E_bit_set(cpustate, 0))
void E_bit_set(dsp56k_core* cpustate, UINT8 value);
#define DSP56K_U_SET() (U_bit_set(cpustate, 1))
#define DSP56K_U_CLEAR() (U_bit_set(cpustate, 0))
void U_bit_set(dsp56k_core* cpustate, UINT8 value);
#define DSP56K_N_SET() (N_bit_set(cpustate, 1))
#define DSP56K_N_CLEAR() (N_bit_set(cpustate, 0))
void N_bit_set(dsp56k_core* cpustate, UINT8 value);
#define DSP56K_Z_SET() (Z_bit_set(cpustate, 1))
#define DSP56K_Z_CLEAR() (Z_bit_set(cpustate, 0))
void Z_bit_set(dsp56k_core* cpustate, UINT8 value);
#define DSP56K_V_SET() (V_bit_set(cpustate, 1))
#define DSP56K_V_CLEAR() (V_bit_set(cpustate, 0))
void V_bit_set(dsp56k_core* cpustate, UINT8 value);
#define DSP56K_C_SET() (C_bit_set(cpustate, 1))
#define DSP56K_C_CLEAR() (C_bit_set(cpustate, 0))
void C_bit_set(dsp56k_core* cpustate, UINT8 value);

// TODO: Maybe some functions for Interrupt Mask and Scaling Mode go here?


/* 1-28 OPERATING MODE REGISTER (OMR) BITS */
//UINT8 CD_bit(dsp56k_core* cpustate);
//UINT8 SD_bit(dsp56k_core* cpustate);
//UINT8 R_bit(dsp56k_core* cpustate);
//UINT8 SA_bit(dsp56k_core* cpustate);
//UINT8 MC_bit(dsp56k_core* cpustate);
UINT8 MB_bit(dsp56k_core* cpustate);
UINT8 MA_bit(dsp56k_core* cpustate);

void CD_bit_set(dsp56k_core* cpustate, UINT8 value);
void SD_bit_set(dsp56k_core* cpustate, UINT8 value);
void R_bit_set(dsp56k_core* cpustate, UINT8 value);
void SA_bit_set(dsp56k_core* cpustate, UINT8 value);
void MC_bit_set(dsp56k_core* cpustate, UINT8 value);
void MB_bit_set(dsp56k_core* cpustate, UINT8 value);
void MA_bit_set(dsp56k_core* cpustate, UINT8 value);

/* 1-27 STACK POINTER (SP) BITS */
UINT8 UF_bit(dsp56k_core* cpustate);
UINT8 SE_bit(dsp56k_core* cpustate);

//void UF_bit_set(dsp56k_core* cpustate, UINT8 value) {};
//void SE_bit_set(dsp56k_core* cpustate, UINT8 value) {};


// HACK - Bootstrap modes
#define BOOTSTRAP_OFF (0)
#define BOOTSTRAP_SSIX (1)
#define BOOTSTRAP_HI (2)


/* PCU IRQ goodies */
void pcu_service_interrupts(dsp56k_core* cpustate);

void dsp56k_irq_table_init(void);
void dsp56k_set_irq_source(UINT8 irq_num, UINT16 iv, const char* source);
int dsp56k_get_irq_index_by_tag(const char* tag);

void dsp56k_add_pending_interrupt(dsp56k_core* cpustate, const char* name);     // Call me to add an interrupt to the queue

void dsp56k_clear_pending_interrupts(dsp56k_core* cpustate);
int dsp56k_count_pending_interrupts(dsp56k_core* cpustate);
void dsp56k_sort_pending_interrupts(dsp56k_core* cpustate, int num);
INT8 dsp56k_get_irq_priority(dsp56k_core* cpustate, int index);

} // namespace DSP56K

#endif
