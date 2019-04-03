// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef MAME_CPU_UNSP_UNSPDEFS_H
#define MAME_CPU_UNSP_UNSPDEFS_H

#pragma once

#define UNSP_LPC            (((m_core->m_r[REG_SR] & 0x3f) << 16) | m_core->m_r[REG_PC])
#define UNSP_LREG_I(reg)    (((m_core->m_r[REG_SR] << 6) & 0x3f0000) | m_core->m_r[reg])

#define UNSP_N  0x0200
#define UNSP_Z  0x0100
#define UNSP_S  0x0080
#define UNSP_C  0x0040

#define UNSP_N_SHIFT    9
#define UNSP_Z_SHIFT    8
#define UNSP_S_SHIFT    7
#define UNSP_C_SHIFT    6

#endif // MAME_CPU_UNSP_UNSPDEFS_H
