#pragma once

#ifndef __M68000_H__
#define __M68000_H__


#include "68307sim.h"
#include "68307bus.h"
#include "68307ser.h"
#include "68307tmu.h"

#include "68340sim.h"
#include "68340dma.h"
#include "68340ser.h"
#include "68340tmu.h"

/* There are 7 levels of interrupt to the 68K.
 * A transition from < 7 to 7 will cause a non-maskable interrupt (NMI).
 */
#define M68K_IRQ_NONE 0
#define M68K_IRQ_1    1
#define M68K_IRQ_2    2
#define M68K_IRQ_3    3
#define M68K_IRQ_4    4
#define M68K_IRQ_5    5
#define M68K_IRQ_6    6
#define M68K_IRQ_7    7

// special input lines
#define M68K_LINE_BUSERROR 16

/* CPU types for use in m68k_set_cpu_type() */
enum
{
	M68K_CPU_TYPE_INVALID,
	M68K_CPU_TYPE_68000,
	M68K_CPU_TYPE_68008,
	M68K_CPU_TYPE_68010,
	M68K_CPU_TYPE_68EC020,
	M68K_CPU_TYPE_68020,
	M68K_CPU_TYPE_68EC030,
	M68K_CPU_TYPE_68030,
	M68K_CPU_TYPE_68EC040,
	M68K_CPU_TYPE_68LC040,
	M68K_CPU_TYPE_68040,
	M68K_CPU_TYPE_SCC68070,
	M68K_CPU_TYPE_68340,
    M68K_CPU_TYPE_COLDFIRE
};

// function codes
enum
{
	M68K_FC_USER_DATA = 1,
	M68K_FC_USER_PROGRAM = 2,
	M68K_FC_SUPERVISOR_DATA = 5,
	M68K_FC_SUPERVISOR_PROGRAM = 6,
	M68K_FC_INTERRUPT = 7
};

/* HMMU enable types for use with m68k_set_hmmu_enable() */
#define M68K_HMMU_DISABLE	0	/* no translation */
#define M68K_HMMU_ENABLE_II	1	/* Mac II style fixed translation */
#define M68K_HMMU_ENABLE_LC	2	/* Mac LC style fixed translation */

/* Special interrupt acknowledge values.
 * Use these as special returns from the interrupt acknowledge callback
 * (specified later in this header).
 */

/* Causes an interrupt autovector (0x18 + interrupt level) to be taken.
 * This happens in a real 68K if VPA or AVEC is asserted during an interrupt
 * acknowledge cycle instead of DTACK.
 */
#define M68K_INT_ACK_AUTOVECTOR    0xffffffff

/* Causes the spurious interrupt vector (0x18) to be taken
 * This happens in a real 68K if BERR is asserted during the interrupt
 * acknowledge cycle (i.e. no devices responded to the acknowledge).
 */
#define M68K_INT_ACK_SPURIOUS      0xfffffffe

enum
{
	/* NOTE: M68K_SP fetches the current SP, be it USP, ISP, or MSP */
	M68K_PC, M68K_SP, M68K_ISP, M68K_USP, M68K_MSP, M68K_SR, M68K_VBR,
	M68K_SFC, M68K_DFC, M68K_CACR, M68K_CAAR, M68K_PREF_ADDR, M68K_PREF_DATA,
	M68K_D0, M68K_D1, M68K_D2, M68K_D3, M68K_D4, M68K_D5, M68K_D6, M68K_D7,
	M68K_A0, M68K_A1, M68K_A2, M68K_A3, M68K_A4, M68K_A5, M68K_A6, M68K_A7,
	M68K_FP0, M68K_FP1, M68K_FP2, M68K_FP3, M68K_FP4, M68K_FP5, M68K_FP6, M68K_FP7,
	M68K_FPSR, M68K_FPCR,

	M68K_GENPC = STATE_GENPC,
	M68K_GENSP = STATE_GENSP,
	M68K_GENPCBASE = STATE_GENPCBASE
};

typedef void (*m68k_bkpt_ack_func)(device_t *device, UINT32 data);
typedef void (*m68k_reset_func)(device_t *device);
typedef void (*m68k_cmpild_func)(device_t *device, UINT32 data, UINT8 reg);
typedef void (*m68k_rte_func)(device_t *device);
typedef int (*m68k_tas_func)(device_t *device);

typedef UINT8 (*m68307_porta_read_callback)(address_space *space, offs_t offset);
typedef void (*m68307_porta_write_callback)(address_space *space, offs_t offset, UINT8 data);
typedef UINT16 (*m68307_portb_read_callback)(address_space *space, offs_t offset, UINT16 mem_mask);
typedef void (*m68307_portb_write_callback)(address_space *space, offs_t offset, UINT16 data, UINT16 mem_mask);



DECLARE_LEGACY_CPU_DEVICE(M68000, m68000);
DECLARE_LEGACY_CPU_DEVICE(M68301, m68301);
DECLARE_LEGACY_CPU_DEVICE(M68307, m68307);
DECLARE_LEGACY_CPU_DEVICE(M68008, m68008);
DECLARE_LEGACY_CPU_DEVICE(M68008PLCC, m68008plcc);
DECLARE_LEGACY_CPU_DEVICE(M68010, m68010);
DECLARE_LEGACY_CPU_DEVICE(M68EC020, m68ec020);
DECLARE_LEGACY_CPU_DEVICE(M68020, m68020);
DECLARE_LEGACY_CPU_DEVICE(M68020PMMU, m68020pmmu);
DECLARE_LEGACY_CPU_DEVICE(M68020HMMU, m68020hmmu);
DECLARE_LEGACY_CPU_DEVICE(M68EC030, m68ec030);
DECLARE_LEGACY_CPU_DEVICE(M68030, m68030);
DECLARE_LEGACY_CPU_DEVICE(M68EC040, m68ec040);
DECLARE_LEGACY_CPU_DEVICE(M68LC040, m68lc040);
DECLARE_LEGACY_CPU_DEVICE(M68040, m68040);
DECLARE_LEGACY_CPU_DEVICE(SCC68070, scc68070);
DECLARE_LEGACY_CPU_DEVICE(M68340, m68340);
DECLARE_LEGACY_CPU_DEVICE(MCF5206E, mcf5206e);


void m68k_set_encrypted_opcode_range(device_t *device, offs_t start, offs_t end);

void m68k_set_hmmu_enable(device_t *device, int enable);

unsigned int m68k_disassemble_raw(char* str_buff, unsigned int pc, const unsigned char* opdata, const unsigned char* argdata, unsigned int cpu_type);

void m68k_set_reset_callback(device_t *device, m68k_reset_func callback);
void m68k_set_cmpild_callback(device_t *device, m68k_cmpild_func callback);
void m68k_set_rte_callback(device_t *device, m68k_rte_func callback);
void m68k_set_tas_callback(device_t *device, m68k_tas_func callback);
UINT16 m68k_get_fc(device_t *device);

void m68307_set_port_callbacks(device_t *device, m68307_porta_read_callback porta_r, m68307_porta_write_callback m_m68307_porta_w, m68307_portb_read_callback portb_r, m68307_portb_write_callback m_m68307_portb_w);
UINT16 m68307_get_cs(device_t *device, offs_t address);
void m68307_set_interrupt(device_t *device, int level, int vector);
void m68307_timer0_interrupt(legacy_cpu_device *cpudev);
void m68307_timer1_interrupt(legacy_cpu_device *cpudev);
void m68307_serial_interrupt(legacy_cpu_device *cpudev);
void m68307_mbus_interrupt(legacy_cpu_device *cpudev);
void m68307_licr2_interrupt(legacy_cpu_device *cpudev);



typedef int (*instruction_hook_t)(device_t *device, offs_t curpc);
void m68k_set_instruction_hook(device_t *device, instruction_hook_t ihook);


#endif /* __M68000_H__ */
