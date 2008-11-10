#pragma once

#ifndef __M68KMAME_H__
#define __M68KMAME_H__

/* ======================================================================== */
/* ============================== MAME STUFF ============================== */
/* ======================================================================== */

#include "debugger.h"
#include "deprecat.h"
#include "m68000.h"

#ifndef __M68KCPU_H__
#define m68ki_cpu_core void
#endif

#define OPT_ON 1
#define OPT_OFF 0

/* Configuration switches (see m68kconf.h for explanation) */
#define M68K_SEPARATE_READS         OPT_ON

#define M68K_EMULATE_TRACE          OPT_OFF

#define M68K_EMULATE_FC             OPT_OFF
#define M68K_SET_FC_CALLBACK(A)

#define M68K_LOG_ENABLE             OPT_OFF
#define M68K_LOG_1010_1111          OPT_OFF
#define M68K_LOG_FILEHANDLE         errorlog

#define M68K_USE_64_BIT             OPT_OFF


extern m68k_memory_interface m68k_memory_intf;
extern offs_t m68k_encrypted_opcode_start[MAX_CPU];
extern offs_t m68k_encrypted_opcode_end[MAX_CPU];

#define m68k_read_memory_8(M, A)				(*m68k_memory_intf.read8)(A)
#define m68k_read_memory_16(M, A)				(*m68k_memory_intf.read16)(A)
#define m68k_read_memory_32(M, A)				(*m68k_memory_intf.read32)(A)

#define m68k_read_immediate_16(M, A)			(*m68k_memory_intf.readimm16)(A)
#define m68k_read_immediate_32(M, A)			m68kx_read_immediate_32(M, A)
#define m68k_read_pcrelative_8(M, A)			m68kx_read_pcrelative_8(M, A)
#define m68k_read_pcrelative_16(M, A)			m68kx_read_pcrelative_16(M, A)
#define m68k_read_pcrelative_32(M, A)			m68kx_read_pcrelative_32(M, A)

#define m68k_read_disassembler_16(M, A)			m68k_read_immediate_16(M, A)
#define m68k_read_disassembler_32(M, A)			m68kx_read_immediate_32(M, A)

#define m68k_write_memory_8(M, A, V)			(*m68k_memory_intf.write8)(A, V)
#define m68k_write_memory_16(M, A, V)			(*m68k_memory_intf.write16)(A, V)
#define m68k_write_memory_32(M, A, V)			(*m68k_memory_intf.write32)(A, V)
#define m68k_write_memory_32_pd(M, A, V)		m68kx_write_memory_32_pd(M, A, V)


INLINE unsigned int m68k_read_immediate_32(m68ki_cpu_core *m68k, unsigned int address);
INLINE unsigned int m68k_read_pcrelative_8(m68ki_cpu_core *m68k, unsigned int address);
INLINE unsigned int m68k_read_pcrelative_16(m68ki_cpu_core *m68k, unsigned int address);
INLINE unsigned int m68k_read_pcrelative_32(m68ki_cpu_core *m68k, unsigned int address);
INLINE void m68k_write_memory_32_pd(m68ki_cpu_core *m68k, unsigned int address, unsigned int value);


INLINE unsigned int m68kx_read_immediate_32(m68ki_cpu_core *m68k, unsigned int address)
{
	return ((m68k_read_immediate_16(m68k, address) << 16) | m68k_read_immediate_16(m68k, (address)+2));
}

INLINE unsigned int m68kx_read_pcrelative_8(m68ki_cpu_core *m68k, unsigned int address)
{
	if (address >= m68k_encrypted_opcode_start[cpunum_get_active()] &&
			address < m68k_encrypted_opcode_end[cpunum_get_active()])
		return ((m68k_read_immediate_16(m68k, address&~1)>>(8*(1-(address & 1))))&0xff);
	else
		return m68k_read_memory_8(m68k, address);
}

INLINE unsigned int m68kx_read_pcrelative_16(m68ki_cpu_core *m68k, unsigned int address)
{
	if (address >= m68k_encrypted_opcode_start[cpunum_get_active()] &&
			address < m68k_encrypted_opcode_end[cpunum_get_active()])
		return m68k_read_immediate_16(m68k, address);
	else
		return m68k_read_memory_16(m68k, address);
}

INLINE unsigned int m68kx_read_pcrelative_32(m68ki_cpu_core *m68k, unsigned int address)
{
	if (address >= m68k_encrypted_opcode_start[cpunum_get_active()] &&
			address < m68k_encrypted_opcode_end[cpunum_get_active()])
		return m68k_read_immediate_32(m68k, address);
	else
		return m68k_read_memory_32(m68k, address);
}


/* Special call to simulate undocumented 68k behavior when move.l with a
 * predecrement destination mode is executed.
 * A real 68k first writes the high word to [address+2], and then writes the
 * low word to [address].
 */
INLINE void m68kx_write_memory_32_pd(m68ki_cpu_core *m68k, unsigned int address, unsigned int value)
{
	(*m68k_memory_intf.write16)(address+2, value>>16);
	(*m68k_memory_intf.write16)(address, value&0xffff);
}


void m68k_set_encrypted_opcode_range(int cpunum, offs_t start, offs_t end);


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* __M68KMAME_H__ */
