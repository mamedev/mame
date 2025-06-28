// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_CPU_SH_SH4REGS_H
#define MAME_CPU_SH_SH4REGS_H

#pragma once

#define MMUCR_LRUI  0xfc000000
#define MMUCR_URB   0x00fc0000
#define MMUCR_URC   0x0000fc00
#define MMUCR_SQMD  0x00000200
#define MMUCR_SV    0x00000100
#define MMUCR_TI    0x00000004
#define MMUCR_AT    0x00000001

/* constants */
#define PVR_SH7091  0x040205c1
#define PVR_SH7750  0x04020500 // from TN-SH7-361B/E
#define PVR_SH7750S 0x04020600
#define PVR_SH7750R 0x04050000
#define PRR_SH7750R 0x00000100
#define PVR_SH7751  0x04110000
#define PVR_SH7751R 0x04050000
#define PRR_SH7751R 0x00000110

#endif // MAME_CPU_SH_SH4REGS_H
