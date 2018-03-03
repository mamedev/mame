// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
#include "emu.h"
#include "arm7.h"
#include "arm7core.h"
#include "arm7help.h"

// this is our master dispatch jump table for THUMB mode, representing [(INSN & 0xffc0) >> 6] bits of the 16-bit decoded instruction
const arm7_cpu_device::arm7thumb_ophandler arm7_cpu_device::thumb_handler[0x40*0x10] =
{
// #define THUMB_SHIFT_R       ((uint16_t)0x0800)
	&arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,
	&arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,
	&arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,
	&arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,
	&arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,
	&arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,
	&arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,
	&arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,     &arm7_cpu_device::tg00_0,
	&arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,
	&arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,
	&arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,
	&arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,
	&arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,
	&arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,
	&arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,
	&arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,     &arm7_cpu_device::tg00_1,
// #define THUMB_INSN_ADDSUB   ((uint16_t)0x0800)   // #define THUMB_ADDSUB_TYPE   ((uint16_t)0x0600)
	&arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,
	&arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,
	&arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,
	&arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,
	&arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,
	&arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,
	&arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,
	&arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,     &arm7_cpu_device::tg01_0,
	&arm7_cpu_device::tg01_10,    &arm7_cpu_device::tg01_10,    &arm7_cpu_device::tg01_10,    &arm7_cpu_device::tg01_10,
	&arm7_cpu_device::tg01_10,    &arm7_cpu_device::tg01_10,    &arm7_cpu_device::tg01_10,    &arm7_cpu_device::tg01_10,
	&arm7_cpu_device::tg01_11,    &arm7_cpu_device::tg01_11,    &arm7_cpu_device::tg01_11,    &arm7_cpu_device::tg01_11,
	&arm7_cpu_device::tg01_11,    &arm7_cpu_device::tg01_11,    &arm7_cpu_device::tg01_11,    &arm7_cpu_device::tg01_11,
	&arm7_cpu_device::tg01_12,    &arm7_cpu_device::tg01_12,    &arm7_cpu_device::tg01_12,    &arm7_cpu_device::tg01_12,
	&arm7_cpu_device::tg01_12,    &arm7_cpu_device::tg01_12,    &arm7_cpu_device::tg01_12,    &arm7_cpu_device::tg01_12,
	&arm7_cpu_device::tg01_13,    &arm7_cpu_device::tg01_13,    &arm7_cpu_device::tg01_13,    &arm7_cpu_device::tg01_13,
	&arm7_cpu_device::tg01_13,    &arm7_cpu_device::tg01_13,    &arm7_cpu_device::tg01_13,    &arm7_cpu_device::tg01_13,
// #define THUMB_INSN_CMP      ((uint16_t)0x0800)
	&arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,
	&arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,
	&arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,
	&arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,
	&arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,
	&arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,
	&arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,
	&arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,     &arm7_cpu_device::tg02_0,
	&arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,
	&arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,
	&arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,
	&arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,
	&arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,
	&arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,
	&arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,
	&arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,     &arm7_cpu_device::tg02_1,
// #define THUMB_INSN_SUB      ((uint16_t)0x0800)
	&arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,
	&arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,
	&arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,
	&arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,
	&arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,
	&arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,
	&arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,
	&arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,     &arm7_cpu_device::tg03_0,
	&arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,
	&arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,
	&arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,
	&arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,
	&arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,
	&arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,
	&arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,
	&arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,     &arm7_cpu_device::tg03_1,
//#define THUMB_GROUP4_TYPE   ((uint16_t)0x0c00)  //#define THUMB_ALUOP_TYPE    ((uint16_t)0x03c0)  // #define THUMB_HIREG_OP      ((uint16_t)0x0300)  // #define THUMB_HIREG_H       ((uint16_t)0x00c0)
	&arm7_cpu_device::tg04_00_00, &arm7_cpu_device::tg04_00_01, &arm7_cpu_device::tg04_00_02, &arm7_cpu_device::tg04_00_03,
	&arm7_cpu_device::tg04_00_04, &arm7_cpu_device::tg04_00_05, &arm7_cpu_device::tg04_00_06, &arm7_cpu_device::tg04_00_07,
	&arm7_cpu_device::tg04_00_08, &arm7_cpu_device::tg04_00_09, &arm7_cpu_device::tg04_00_0a, &arm7_cpu_device::tg04_00_0b,
	&arm7_cpu_device::tg04_00_0c, &arm7_cpu_device::tg04_00_0d, &arm7_cpu_device::tg04_00_0e, &arm7_cpu_device::tg04_00_0f,
	&arm7_cpu_device::tg04_01_00, &arm7_cpu_device::tg04_01_01, &arm7_cpu_device::tg04_01_02, &arm7_cpu_device::tg04_01_03,
	&arm7_cpu_device::tg04_01_10, &arm7_cpu_device::tg04_01_11, &arm7_cpu_device::tg04_01_12, &arm7_cpu_device::tg04_01_13,
	&arm7_cpu_device::tg04_01_20, &arm7_cpu_device::tg04_01_21, &arm7_cpu_device::tg04_01_22, &arm7_cpu_device::tg04_01_23,
	&arm7_cpu_device::tg04_01_30, &arm7_cpu_device::tg04_01_31, &arm7_cpu_device::tg04_01_32, &arm7_cpu_device::tg04_01_33,
	&arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,
	&arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,
	&arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,
	&arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,
	&arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,
	&arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,
	&arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,
	&arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,  &arm7_cpu_device::tg04_0203,
//#define THUMB_GROUP5_TYPE   ((uint16_t)0x0e00)
	&arm7_cpu_device::tg05_0,     &arm7_cpu_device::tg05_0,     &arm7_cpu_device::tg05_0,     &arm7_cpu_device::tg05_0,
	&arm7_cpu_device::tg05_0,     &arm7_cpu_device::tg05_0,     &arm7_cpu_device::tg05_0,     &arm7_cpu_device::tg05_0,
	&arm7_cpu_device::tg05_1,     &arm7_cpu_device::tg05_1,     &arm7_cpu_device::tg05_1,     &arm7_cpu_device::tg05_1,
	&arm7_cpu_device::tg05_1,     &arm7_cpu_device::tg05_1,     &arm7_cpu_device::tg05_1,     &arm7_cpu_device::tg05_1,
	&arm7_cpu_device::tg05_2,     &arm7_cpu_device::tg05_2,     &arm7_cpu_device::tg05_2,     &arm7_cpu_device::tg05_2,
	&arm7_cpu_device::tg05_2,     &arm7_cpu_device::tg05_2,     &arm7_cpu_device::tg05_2,     &arm7_cpu_device::tg05_2,
	&arm7_cpu_device::tg05_3,     &arm7_cpu_device::tg05_3,     &arm7_cpu_device::tg05_3,     &arm7_cpu_device::tg05_3,
	&arm7_cpu_device::tg05_3,     &arm7_cpu_device::tg05_3,     &arm7_cpu_device::tg05_3,     &arm7_cpu_device::tg05_3,
	&arm7_cpu_device::tg05_4,     &arm7_cpu_device::tg05_4,     &arm7_cpu_device::tg05_4,     &arm7_cpu_device::tg05_4,
	&arm7_cpu_device::tg05_4,     &arm7_cpu_device::tg05_4,     &arm7_cpu_device::tg05_4,     &arm7_cpu_device::tg05_4,
	&arm7_cpu_device::tg05_5,     &arm7_cpu_device::tg05_5,     &arm7_cpu_device::tg05_5,     &arm7_cpu_device::tg05_5,
	&arm7_cpu_device::tg05_5,     &arm7_cpu_device::tg05_5,     &arm7_cpu_device::tg05_5,     &arm7_cpu_device::tg05_5,
	&arm7_cpu_device::tg05_6,     &arm7_cpu_device::tg05_6,     &arm7_cpu_device::tg05_6,     &arm7_cpu_device::tg05_6,
	&arm7_cpu_device::tg05_6,     &arm7_cpu_device::tg05_6,     &arm7_cpu_device::tg05_6,     &arm7_cpu_device::tg05_6,
	&arm7_cpu_device::tg05_7,     &arm7_cpu_device::tg05_7,     &arm7_cpu_device::tg05_7,     &arm7_cpu_device::tg05_7,
	&arm7_cpu_device::tg05_7,     &arm7_cpu_device::tg05_7,     &arm7_cpu_device::tg05_7,     &arm7_cpu_device::tg05_7,
//#define THUMB_LSOP_L        ((uint16_t)0x0800)
	&arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,
	&arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,
	&arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,
	&arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,
	&arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,
	&arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,
	&arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,
	&arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,     &arm7_cpu_device::tg06_0,
	&arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,
	&arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,
	&arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,
	&arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,
	&arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,
	&arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,
	&arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,
	&arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,     &arm7_cpu_device::tg06_1,
//#define THUMB_LSOP_L        ((uint16_t)0x0800)
	&arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,
	&arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,
	&arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,
	&arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,
	&arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,
	&arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,
	&arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,
	&arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,     &arm7_cpu_device::tg07_0,
	&arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,
	&arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,
	&arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,
	&arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,
	&arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,
	&arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,
	&arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,
	&arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,     &arm7_cpu_device::tg07_1,
// #define THUMB_HALFOP_L      ((uint16_t)0x0800)
	&arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,
	&arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,
	&arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,
	&arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,
	&arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,
	&arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,
	&arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,
	&arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,     &arm7_cpu_device::tg08_0,
	&arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,
	&arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,
	&arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,
	&arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,
	&arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,
	&arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,
	&arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,
	&arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,     &arm7_cpu_device::tg08_1,
// #define THUMB_STACKOP_L     ((uint16_t)0x0800)
	&arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,
	&arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,
	&arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,
	&arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,
	&arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,
	&arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,
	&arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,
	&arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,     &arm7_cpu_device::tg09_0,
	&arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,
	&arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,
	&arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,
	&arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,
	&arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,
	&arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,
	&arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,
	&arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,     &arm7_cpu_device::tg09_1,
// #define THUMB_RELADDR_SP    ((uint16_t)0x0800)
	&arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,
	&arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,
	&arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,
	&arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,
	&arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,
	&arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,
	&arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,
	&arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,     &arm7_cpu_device::tg0a_0,
	&arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,
	&arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,
	&arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,
	&arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,
	&arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,
	&arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,
	&arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,
	&arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,     &arm7_cpu_device::tg0a_1,
// #define THUMB_STACKOP_TYPE  ((uint16_t)0x0f00)
	&arm7_cpu_device::tg0b_0,     &arm7_cpu_device::tg0b_0,     &arm7_cpu_device::tg0b_0,     &arm7_cpu_device::tg0b_0,
	&arm7_cpu_device::tg0b_1,     &arm7_cpu_device::tg0b_1,     &arm7_cpu_device::tg0b_1,     &arm7_cpu_device::tg0b_1,
	&arm7_cpu_device::tg0b_2,     &arm7_cpu_device::tg0b_2,     &arm7_cpu_device::tg0b_2,     &arm7_cpu_device::tg0b_2,
	&arm7_cpu_device::tg0b_3,     &arm7_cpu_device::tg0b_3,     &arm7_cpu_device::tg0b_3,     &arm7_cpu_device::tg0b_3,
	&arm7_cpu_device::tg0b_4,     &arm7_cpu_device::tg0b_4,     &arm7_cpu_device::tg0b_4,     &arm7_cpu_device::tg0b_4,
	&arm7_cpu_device::tg0b_5,     &arm7_cpu_device::tg0b_5,     &arm7_cpu_device::tg0b_5,     &arm7_cpu_device::tg0b_5,
	&arm7_cpu_device::tg0b_6,     &arm7_cpu_device::tg0b_6,     &arm7_cpu_device::tg0b_6,     &arm7_cpu_device::tg0b_6,
	&arm7_cpu_device::tg0b_7,     &arm7_cpu_device::tg0b_7,     &arm7_cpu_device::tg0b_7,     &arm7_cpu_device::tg0b_7,
	&arm7_cpu_device::tg0b_8,     &arm7_cpu_device::tg0b_8,     &arm7_cpu_device::tg0b_8,     &arm7_cpu_device::tg0b_8,
	&arm7_cpu_device::tg0b_9,     &arm7_cpu_device::tg0b_9,     &arm7_cpu_device::tg0b_9,     &arm7_cpu_device::tg0b_9,
	&arm7_cpu_device::tg0b_a,     &arm7_cpu_device::tg0b_a,     &arm7_cpu_device::tg0b_a,     &arm7_cpu_device::tg0b_a,
	&arm7_cpu_device::tg0b_b,     &arm7_cpu_device::tg0b_b,     &arm7_cpu_device::tg0b_b,     &arm7_cpu_device::tg0b_b,
	&arm7_cpu_device::tg0b_c,     &arm7_cpu_device::tg0b_c,     &arm7_cpu_device::tg0b_c,     &arm7_cpu_device::tg0b_c,
	&arm7_cpu_device::tg0b_d,     &arm7_cpu_device::tg0b_d,     &arm7_cpu_device::tg0b_d,     &arm7_cpu_device::tg0b_d,
	&arm7_cpu_device::tg0b_e,     &arm7_cpu_device::tg0b_e,     &arm7_cpu_device::tg0b_e,     &arm7_cpu_device::tg0b_e,
	&arm7_cpu_device::tg0b_f,     &arm7_cpu_device::tg0b_f,     &arm7_cpu_device::tg0b_f,     &arm7_cpu_device::tg0b_f,
// #define THUMB_MULTLS        ((uint16_t)0x0800)
	&arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,
	&arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,
	&arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,
	&arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,
	&arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,
	&arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,
	&arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,
	&arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,     &arm7_cpu_device::tg0c_0,
	&arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,
	&arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,
	&arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,
	&arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,
	&arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,
	&arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,
	&arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,
	&arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,     &arm7_cpu_device::tg0c_1,
// #define THUMB_COND_TYPE     ((uint16_t)0x0f00)
	&arm7_cpu_device::tg0d_0,     &arm7_cpu_device::tg0d_0,     &arm7_cpu_device::tg0d_0,     &arm7_cpu_device::tg0d_0,
	&arm7_cpu_device::tg0d_1,     &arm7_cpu_device::tg0d_1,     &arm7_cpu_device::tg0d_1,     &arm7_cpu_device::tg0d_1,
	&arm7_cpu_device::tg0d_2,     &arm7_cpu_device::tg0d_2,     &arm7_cpu_device::tg0d_2,     &arm7_cpu_device::tg0d_2,
	&arm7_cpu_device::tg0d_3,     &arm7_cpu_device::tg0d_3,     &arm7_cpu_device::tg0d_3,     &arm7_cpu_device::tg0d_3,
	&arm7_cpu_device::tg0d_4,     &arm7_cpu_device::tg0d_4,     &arm7_cpu_device::tg0d_4,     &arm7_cpu_device::tg0d_4,
	&arm7_cpu_device::tg0d_5,     &arm7_cpu_device::tg0d_5,     &arm7_cpu_device::tg0d_5,     &arm7_cpu_device::tg0d_5,
	&arm7_cpu_device::tg0d_6,     &arm7_cpu_device::tg0d_6,     &arm7_cpu_device::tg0d_6,     &arm7_cpu_device::tg0d_6,
	&arm7_cpu_device::tg0d_7,     &arm7_cpu_device::tg0d_7,     &arm7_cpu_device::tg0d_7,     &arm7_cpu_device::tg0d_7,
	&arm7_cpu_device::tg0d_8,     &arm7_cpu_device::tg0d_8,     &arm7_cpu_device::tg0d_8,     &arm7_cpu_device::tg0d_8,
	&arm7_cpu_device::tg0d_9,     &arm7_cpu_device::tg0d_9,     &arm7_cpu_device::tg0d_9,     &arm7_cpu_device::tg0d_9,
	&arm7_cpu_device::tg0d_a,     &arm7_cpu_device::tg0d_a,     &arm7_cpu_device::tg0d_a,     &arm7_cpu_device::tg0d_a,
	&arm7_cpu_device::tg0d_b,     &arm7_cpu_device::tg0d_b,     &arm7_cpu_device::tg0d_b,     &arm7_cpu_device::tg0d_b,
	&arm7_cpu_device::tg0d_c,     &arm7_cpu_device::tg0d_c,     &arm7_cpu_device::tg0d_c,     &arm7_cpu_device::tg0d_c,
	&arm7_cpu_device::tg0d_d,     &arm7_cpu_device::tg0d_d,     &arm7_cpu_device::tg0d_d,     &arm7_cpu_device::tg0d_d,
	&arm7_cpu_device::tg0d_e,     &arm7_cpu_device::tg0d_e,     &arm7_cpu_device::tg0d_e,     &arm7_cpu_device::tg0d_e,
	&arm7_cpu_device::tg0d_f,     &arm7_cpu_device::tg0d_f,     &arm7_cpu_device::tg0d_f,     &arm7_cpu_device::tg0d_f,
// #define THUMB_BLOP_LO       ((uint16_t)0x0800)
	&arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,
	&arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,
	&arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,
	&arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,
	&arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,
	&arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,
	&arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,
	&arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,     &arm7_cpu_device::tg0e_0,
	&arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,
	&arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,
	&arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,
	&arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,
	&arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,
	&arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,
	&arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,
	&arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,     &arm7_cpu_device::tg0e_1,
// #define THUMB_BLOP_LO       ((uint16_t)0x0800)
	&arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,
	&arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,
	&arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,
	&arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,
	&arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,
	&arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,
	&arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,
	&arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,     &arm7_cpu_device::tg0f_0,
	&arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,
	&arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,
	&arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,
	&arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,
	&arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,
	&arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,
	&arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,
	&arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,     &arm7_cpu_device::tg0f_1,
};

	/* Shift operations */

void arm7_cpu_device::tg00_0(uint32_t pc, uint32_t op) /* Shift left */
{
	uint32_t rs, rd, rrs;
	int32_t offs;

	set_cpsr(GET_CPSR & ~(N_MASK | Z_MASK));

	rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rrs = GetRegister(rs);
	offs = (op & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
	if (offs != 0)
	{
		SetRegister(rd, rrs << offs);
		if (rrs & (1 << (31 - (offs - 1))))
		{
			set_cpsr(GET_CPSR | C_MASK);
		}
		else
		{
			set_cpsr(GET_CPSR & ~C_MASK);
		}
	}
	else
	{
		SetRegister(rd, rrs);
	}
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
	R15 += 2;
}

void arm7_cpu_device::tg00_1(uint32_t pc, uint32_t op) /* Shift right */
{
	uint32_t rs, rd, rrs;
	int32_t offs;

	rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rrs = GetRegister(rs);
	offs = (op & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
	if (offs != 0)
	{
		SetRegister(rd, rrs >> offs);
		if (rrs & (1 << (offs - 1)))
		{
			set_cpsr(GET_CPSR | C_MASK);
		}
		else
		{
			set_cpsr(GET_CPSR & ~C_MASK);
		}
	}
	else
	{
		SetRegister(rd, 0);
		if (rrs & 0x80000000)
		{
			set_cpsr(GET_CPSR | C_MASK);
		}
		else
		{
			set_cpsr(GET_CPSR & ~C_MASK);
		}
	}
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
	R15 += 2;
}

	/* Arithmetic */

void arm7_cpu_device::tg01_0(uint32_t pc, uint32_t op)
{
	uint32_t rs, rd, rrs;
	int32_t offs;
	/* ASR.. */
	//if (op & THUMB_SHIFT_R) /* Shift right */
	{
		rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
		rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
		rrs = GetRegister(rs);
		offs = (op & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
		if (offs == 0)
		{
			offs = 32;
		}
		if (offs >= 32)
		{
			if (rrs >> 31)
			{
				set_cpsr(GET_CPSR | C_MASK);
			}
			else
			{
				set_cpsr(GET_CPSR & ~C_MASK);
			}
			SetRegister(rd, (rrs & 0x80000000) ? 0xFFFFFFFF : 0x00000000);
		}
		else
		{
			if ((rrs >> (offs - 1)) & 1)
			{
				set_cpsr(GET_CPSR | C_MASK);
			}
			else
			{
				set_cpsr(GET_CPSR & ~C_MASK);
			}
			SetRegister(rd,
							(rrs & 0x80000000)
							? ((0xFFFFFFFF << (32 - offs)) | (rrs >> offs))
							: (rrs >> offs));
		}
		set_cpsr(GET_CPSR & ~(N_MASK | Z_MASK));
		set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
		R15 += 2;
	}
}

void arm7_cpu_device::tg01_10(uint32_t pc, uint32_t op)  /* ADD Rd, Rs, Rn */
{
	uint32_t rn = GetRegister((op & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT);
	uint32_t rs = GetRegister((op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SetRegister(rd, rs + rn);
	HandleThumbALUAddFlags(GetRegister(rd), rs, rn);

}

void arm7_cpu_device::tg01_11(uint32_t pc, uint32_t op) /* SUB Rd, Rs, Rn */
{
	uint32_t rn = GetRegister((op & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT);
	uint32_t rs = GetRegister((op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SetRegister(rd, rs - rn);
	HandleThumbALUSubFlags(GetRegister(rd), rs, rn);

}

void arm7_cpu_device::tg01_12(uint32_t pc, uint32_t op) /* ADD Rd, Rs, #imm */
{
	uint32_t imm = (op & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT;
	uint32_t rs = GetRegister((op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SetRegister(rd, rs + imm);
	HandleThumbALUAddFlags(GetRegister(rd), rs, imm);

}

void arm7_cpu_device::tg01_13(uint32_t pc, uint32_t op) /* SUB Rd, Rs, #imm */
{
	uint32_t imm = (op & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT;
	uint32_t rs = GetRegister((op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SetRegister(rd, rs - imm);
	HandleThumbALUSubFlags(GetRegister(rd), rs,imm);

}

	/* CMP / MOV */

void arm7_cpu_device::tg02_0(uint32_t pc, uint32_t op)
{
	uint32_t rd = (op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT;
	uint32_t op2 = (op & THUMB_INSN_IMM);
	SetRegister(rd, op2);
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
	R15 += 2;
}

void arm7_cpu_device::tg02_1(uint32_t pc, uint32_t op)
{
	uint32_t rn = GetRegister((op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
	uint32_t op2 = op & THUMB_INSN_IMM;
	uint32_t rd = rn - op2;
	HandleThumbALUSubFlags(rd, rn, op2);
}

	/* ADD/SUB immediate */

void arm7_cpu_device::tg03_0(uint32_t pc, uint32_t op) /* ADD Rd, #Offset8 */
{
	uint32_t rn = GetRegister((op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
	uint32_t op2 = op & THUMB_INSN_IMM;
	uint32_t rd = rn + op2;
	SetRegister((op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, rd);
	HandleThumbALUAddFlags(rd, rn, op2);
}

void arm7_cpu_device::tg03_1(uint32_t pc, uint32_t op) /* SUB Rd, #Offset8 */
{
	uint32_t rn = GetRegister((op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
	uint32_t op2 = op & THUMB_INSN_IMM;
	uint32_t rd = rn - op2;
	SetRegister((op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, rd);
	HandleThumbALUSubFlags(rd, rn, op2);
}

	/* Rd & Rm instructions */

void arm7_cpu_device::tg04_00_00(uint32_t pc, uint32_t op) /* AND Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SetRegister(rd, GetRegister(rd) & GetRegister(rs));
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
	R15 += 2;
}

void arm7_cpu_device::tg04_00_01(uint32_t pc, uint32_t op) /* EOR Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SetRegister(rd, GetRegister(rd) ^ GetRegister(rs));
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
	R15 += 2;
}

void arm7_cpu_device::tg04_00_02(uint32_t pc, uint32_t op) /* LSL Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	uint32_t rrd = GetRegister(rd);
	int32_t offs = GetRegister(rs) & 0x000000ff;
	if (offs > 0)
	{
		if (offs < 32)
		{
			SetRegister(rd, rrd << offs);
			if (rrd & (1 << (31 - (offs - 1))))
			{
				set_cpsr(GET_CPSR | C_MASK);
			}
			else
			{
				set_cpsr(GET_CPSR & ~C_MASK);
			}
		}
		else if (offs == 32)
		{
			SetRegister(rd, 0);
			if (rrd & 1)
			{
				set_cpsr(GET_CPSR | C_MASK);
			}
			else
			{
				set_cpsr(GET_CPSR & ~C_MASK);
			}
		}
		else
		{
			SetRegister(rd, 0);
			set_cpsr(GET_CPSR & ~C_MASK);
		}
	}
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
	R15 += 2;
}

void arm7_cpu_device::tg04_00_03(uint32_t pc, uint32_t op) /* LSR Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	uint32_t rrd = GetRegister(rd);
	int32_t offs = GetRegister(rs) & 0x000000ff;
	if (offs >  0)
	{
		if (offs < 32)
		{
			SetRegister(rd, rrd >> offs);
			if (rrd & (1 << (offs - 1)))
			{
				set_cpsr(GET_CPSR | C_MASK);
			}
			else
			{
				set_cpsr(GET_CPSR & ~C_MASK);
			}
		}
		else if (offs == 32)
		{
			SetRegister(rd, 0);
			if (rrd & 0x80000000)
			{
				set_cpsr(GET_CPSR | C_MASK);
			}
			else
			{
				set_cpsr(GET_CPSR & ~C_MASK);
			}
		}
		else
		{
			SetRegister(rd, 0);
			set_cpsr(GET_CPSR & ~C_MASK);
		}
	}
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
	R15 += 2;
}

void arm7_cpu_device::tg04_00_04(uint32_t pc, uint32_t op) /* ASR Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	uint32_t rrs = GetRegister(rs)&0xff;
	uint32_t rrd = GetRegister(rd);
	if (rrs != 0)
	{
		if (rrs >= 32)
		{
			if (rrd >> 31)
			{
				set_cpsr(GET_CPSR | C_MASK);
			}
			else
			{
				set_cpsr(GET_CPSR & ~C_MASK);
			}
			SetRegister(rd, (GetRegister(rd) & 0x80000000) ? 0xFFFFFFFF : 0x00000000);
		}
		else
		{
			if ((rrd >> (rrs-1)) & 1)
			{
				set_cpsr(GET_CPSR | C_MASK);
			}
			else
			{
				set_cpsr(GET_CPSR & ~C_MASK);
			}
			SetRegister(rd, (rrd & 0x80000000)
							? ((0xFFFFFFFF << (32 - rrs)) | (rrd >> rrs))
							: (rrd >> rrs));
		}
	}
	set_cpsr(GET_CPSR & ~(N_MASK | Z_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
	R15 += 2;
}

void arm7_cpu_device::tg04_00_05(uint32_t pc, uint32_t op) /* ADC Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	uint32_t op2 = (GET_CPSR & C_MASK) ? 1 : 0;
	uint32_t rn = GetRegister(rd) + GetRegister(rs) + op2;
	HandleThumbALUAddFlags(rn, GetRegister(rd), (GetRegister(rs))); // ?
	SetRegister(rd, rn);
}

void arm7_cpu_device::tg04_00_06(uint32_t pc, uint32_t op)  /* SBC Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	uint32_t op2 = (GET_CPSR & C_MASK) ? 0 : 1;
	uint32_t rn = GetRegister(rd) - GetRegister(rs) - op2;
	HandleThumbALUSubFlags(rn, GetRegister(rd), (GetRegister(rs))); //?
	SetRegister(rd, rn);
}

void arm7_cpu_device::tg04_00_07(uint32_t pc, uint32_t op) /* ROR Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	uint32_t rrd = GetRegister(rd);
	uint32_t imm = GetRegister(rs) & 0x0000001f;
	SetRegister(rd, (rrd >> imm) | (rrd << (32 - imm)));
	if (rrd & (1 << (imm - 1)))
	{
		set_cpsr(GET_CPSR | C_MASK);
	}
	else
	{
		set_cpsr(GET_CPSR & ~C_MASK);
	}
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
	R15 += 2;
}

void arm7_cpu_device::tg04_00_08(uint32_t pc, uint32_t op) /* TST Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd) & GetRegister(rs)));
	R15 += 2;
}

void arm7_cpu_device::tg04_00_09(uint32_t pc, uint32_t op) /* NEG Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	uint32_t rrs = GetRegister(rs);
	SetRegister(rd, 0 - rrs);
	HandleThumbALUSubFlags(GetRegister(rd), 0, rrs);
}

void arm7_cpu_device::tg04_00_0a(uint32_t pc, uint32_t op) /* CMP Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	uint32_t rn = GetRegister(rd) - GetRegister(rs);
	HandleThumbALUSubFlags(rn, GetRegister(rd), GetRegister(rs));
}

void arm7_cpu_device::tg04_00_0b(uint32_t pc, uint32_t op) /* CMN Rd, Rs - check flags, add dasm */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	uint32_t rn = GetRegister(rd) + GetRegister(rs);
	HandleThumbALUAddFlags(rn, GetRegister(rd), GetRegister(rs));
}

void arm7_cpu_device::tg04_00_0c(uint32_t pc, uint32_t op) /* ORR Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SetRegister(rd, GetRegister(rd) | GetRegister(rs));
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
	R15 += 2;
}

void arm7_cpu_device::tg04_00_0d(uint32_t pc, uint32_t op) /* MUL Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	uint32_t rn = GetRegister(rd) * GetRegister(rs);
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	SetRegister(rd, rn);
	set_cpsr(GET_CPSR | HandleALUNZFlags(rn));
	R15 += 2;
}

void arm7_cpu_device::tg04_00_0e(uint32_t pc, uint32_t op) /* BIC Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SetRegister(rd, GetRegister(rd) & (~GetRegister(rs)));
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
	R15 += 2;
}

void arm7_cpu_device::tg04_00_0f(uint32_t pc, uint32_t op) /* MVN Rd, Rs */
{
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	uint32_t op2 = GetRegister(rs);
	SetRegister(rd, ~op2);
	set_cpsr(GET_CPSR & ~(Z_MASK | N_MASK));
	set_cpsr(GET_CPSR | HandleALUNZFlags(GetRegister(rd)));
	R15 += 2;
}

/* ADD Rd, Rs group */

void arm7_cpu_device::tg04_01_00(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: G4-1-0 Undefined Thumb instruction: %04x %x\n", pc, op, (op & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT);
}

void arm7_cpu_device::tg04_01_01(uint32_t pc, uint32_t op) /* ADD Rd, HRs */
{
	uint32_t rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	uint32_t rd = op & THUMB_HIREG_RD;
	SetRegister(rd, GetRegister(rd) + GetRegister(rs+8));
	// emulate the effects of pre-fetch
	if (rs == 7)
	{
		SetRegister(rd, GetRegister(rd) + 4);
	}

	R15 += 2;
}

void arm7_cpu_device::tg04_01_02(uint32_t pc, uint32_t op) /* ADD HRd, Rs */
{
	uint32_t rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	uint32_t rd = op & THUMB_HIREG_RD;
	SetRegister(rd+8, GetRegister(rd+8) + GetRegister(rs));
	if (rd == 7)
	{
		R15 += 2;
	}

	R15 += 2;
}

void arm7_cpu_device::tg04_01_03(uint32_t pc, uint32_t op) /* Add HRd, HRs */
{
	uint32_t rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	uint32_t rd = op & THUMB_HIREG_RD;
	SetRegister(rd+8, GetRegister(rd+8) + GetRegister(rs+8));
	// emulate the effects of pre-fetch
	if (rs == 7)
	{
		SetRegister(rd+8, GetRegister(rd+8) + 4);
	}
	if (rd == 7)
	{
		R15 += 2;
	}

	R15 += 2;
}

void arm7_cpu_device::tg04_01_10(uint32_t pc, uint32_t op)  /* CMP Rd, Rs */
{
	uint32_t rs = GetRegister(((op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT));
	uint32_t rd = GetRegister(op & THUMB_HIREG_RD);
	uint32_t rn = rd - rs;
	HandleThumbALUSubFlags(rn, rd, rs);
}

void arm7_cpu_device::tg04_01_11(uint32_t pc, uint32_t op) /* CMP Rd, Hs */
{
	uint32_t rs = GetRegister(((op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
	uint32_t rd = GetRegister(op & THUMB_HIREG_RD);
	uint32_t rn = rd - rs;
	HandleThumbALUSubFlags(rn, rd, rs);
}

void arm7_cpu_device::tg04_01_12(uint32_t pc, uint32_t op) /* CMP Hd, Rs */
{
	uint32_t rs = GetRegister(((op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT));
	uint32_t rd = GetRegister((op & THUMB_HIREG_RD) + 8);
	uint32_t rn = rd - rs;
	HandleThumbALUSubFlags(rn, rd, rs);
}

void arm7_cpu_device::tg04_01_13(uint32_t pc, uint32_t op) /* CMP Hd, Hs */
{
	uint32_t rs = GetRegister(((op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
	uint32_t rd = GetRegister((op & THUMB_HIREG_RD) + 8);
	uint32_t rn = rd - rs;
	HandleThumbALUSubFlags(rn, rd, rs);
}

/* MOV group */

// "The action of H1 = 0, H2 = 0 for Op = 00 (ADD), Op = 01 (CMP) and Op = 10 (MOV) is undefined, and should not be used."
void arm7_cpu_device::tg04_01_20(uint32_t pc, uint32_t op) /* MOV Rd, Rs (undefined) */
{
	uint32_t rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	uint32_t rd = op & THUMB_HIREG_RD;
	SetRegister(rd, GetRegister(rs));
	R15 += 2;
}

void arm7_cpu_device::tg04_01_21(uint32_t pc, uint32_t op) /* MOV Rd, Hs */
{
	uint32_t rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	uint32_t rd = op & THUMB_HIREG_RD;
	SetRegister(rd, GetRegister(rs + 8));
	if (rs == 7)
	{
		SetRegister(rd, GetRegister(rd) + 4);
	}
	R15 += 2;
}

void arm7_cpu_device::tg04_01_22(uint32_t pc, uint32_t op) /* MOV Hd, Rs */
{
	uint32_t rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	uint32_t rd = op & THUMB_HIREG_RD;
	SetRegister(rd + 8, GetRegister(rs));
	if (rd != 7)
	{
		R15 += 2;
	}
	else
	{
		R15 &= ~1;
	}
}

void arm7_cpu_device::tg04_01_23(uint32_t pc, uint32_t op) /* MOV Hd, Hs */
{
	uint32_t rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	uint32_t rd = op & THUMB_HIREG_RD;
	if (rs == 7)
	{
		SetRegister(rd + 8, GetRegister(rs+8)+4);
	}
	else
	{
		SetRegister(rd + 8, GetRegister(rs+8));
	}
	if (rd != 7)
	{
		R15 += 2;
	}
	else
	{
		R15 &= ~1;
	}
}

void arm7_cpu_device::tg04_01_30(uint32_t pc, uint32_t op)
{
	uint32_t rd = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	uint32_t addr = GetRegister(rd);
	if (addr & 1)
	{
		addr &= ~1;
	}
	else
	{
		set_cpsr(GET_CPSR & ~T_MASK);
		if (addr & 2)
		{
			addr += 2;
		}
	}
	R15 = addr;
}

void arm7_cpu_device::tg04_01_31(uint32_t pc, uint32_t op)
{
	uint32_t rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	uint32_t addr = GetRegister(rs+8);
	if (rs == 7)
	{
		addr += 2;
	}
	if (addr & 1)
	{
		addr &= ~1;
	}
	else
	{
		set_cpsr(GET_CPSR & ~T_MASK);
		if (addr & 2)
		{
			addr += 2;
		}
	}
	R15 = addr;
}

/* BLX */
void arm7_cpu_device::tg04_01_32(uint32_t pc, uint32_t op)
{
	uint32_t addr = GetRegister((op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT);
	SetRegister(14, (R15 + 2) | 1);

	// are we also switching to ARM mode?
	if (!(addr & 1))
	{
		set_cpsr(GET_CPSR & ~T_MASK);
		if (addr & 2)
		{
			addr += 2;
		}
	}
	else
	{
		addr &= ~1;
	}

	R15 = addr;
}

void arm7_cpu_device::tg04_01_33(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: G4-3 Undefined Thumb instruction: %04x\n", pc, op);
}

void arm7_cpu_device::tg04_0203(uint32_t pc, uint32_t op)
{
	uint32_t readword = READ32((R15 & ~2) + 4 + ((op & THUMB_INSN_IMM) << 2));
	SetRegister((op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, readword);
	R15 += 2;
}

/* LDR* STR* group */

void arm7_cpu_device::tg05_0(uint32_t pc, uint32_t op)  /* STR Rd, [Rn, Rm] */
{
	uint32_t rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	uint32_t rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	uint32_t rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	uint32_t addr = GetRegister(rn) + GetRegister(rm);
	WRITE32(addr, GetRegister(rd));
	R15 += 2;
}

void arm7_cpu_device::tg05_1(uint32_t pc, uint32_t op)  /* STRH Rd, [Rn, Rm] */
{
	uint32_t rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	uint32_t rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	uint32_t rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	uint32_t addr = GetRegister(rn) + GetRegister(rm);
	WRITE16(addr, GetRegister(rd));
	R15 += 2;
}

void arm7_cpu_device::tg05_2(uint32_t pc, uint32_t op)  /* STRB Rd, [Rn, Rm] */
{
	uint32_t rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	uint32_t rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	uint32_t rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	uint32_t addr = GetRegister(rn) + GetRegister(rm);
	WRITE8(addr, GetRegister(rd));
	R15 += 2;
}

void arm7_cpu_device::tg05_3(uint32_t pc, uint32_t op)  /* LDSB Rd, [Rn, Rm] todo, add dasm */
{
	uint32_t rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	uint32_t rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	uint32_t rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	uint32_t addr = GetRegister(rn) + GetRegister(rm);
	uint32_t op2 = READ8(addr);
	if (op2 & 0x00000080)
	{
		op2 |= 0xffffff00;
	}
	SetRegister(rd, op2);
	R15 += 2;
}

void arm7_cpu_device::tg05_4(uint32_t pc, uint32_t op)  /* LDR Rd, [Rn, Rm] */
{
	uint32_t rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	uint32_t rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	uint32_t rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	uint32_t addr = GetRegister(rn) + GetRegister(rm);
	uint32_t op2 = READ32(addr);
	SetRegister(rd, op2);
	R15 += 2;
}

void arm7_cpu_device::tg05_5(uint32_t pc, uint32_t op)  /* LDRH Rd, [Rn, Rm] */
{
	uint32_t rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	uint32_t rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	uint32_t rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	uint32_t addr = GetRegister(rn) + GetRegister(rm);
	uint32_t op2 = READ16(addr);
	SetRegister(rd, op2);
	R15 += 2;
}

void arm7_cpu_device::tg05_6(uint32_t pc, uint32_t op)  /* LDRB Rd, [Rn, Rm] */
{
	uint32_t rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	uint32_t rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	uint32_t rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	uint32_t addr = GetRegister(rn) + GetRegister(rm);
	uint32_t op2 = READ8(addr);
	SetRegister(rd, op2);
	R15 += 2;
}

void arm7_cpu_device::tg05_7(uint32_t pc, uint32_t op)  /* LDSH Rd, [Rn, Rm] */
{
	uint32_t rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	uint32_t rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	uint32_t rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	uint32_t addr = GetRegister(rn) + GetRegister(rm);
	int32_t op2 = (int32_t)(int16_t)(uint16_t)READ16(addr & ~1);
	if ((addr & 1) && m_archRev < 5)
		op2 >>= 8;
	SetRegister(rd, op2);
	R15 += 2;
}

	/* Word Store w/ Immediate Offset */

void arm7_cpu_device::tg06_0(uint32_t pc, uint32_t op) /* Store */
{
	uint32_t rn = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = op & THUMB_ADDSUB_RD;
	int32_t offs = ((op & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT) << 2;
	WRITE32(GetRegister(rn) + offs, GetRegister(rd));
	R15 += 2;
}

void arm7_cpu_device::tg06_1(uint32_t pc, uint32_t op) /* Load */
{
	uint32_t rn = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = op & THUMB_ADDSUB_RD;
	int32_t offs = ((op & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT) << 2;
	SetRegister(rd, READ32(GetRegister(rn) + offs)); // fix
	R15 += 2;
}

/* Byte Store w/ Immeidate Offset */

void arm7_cpu_device::tg07_0(uint32_t pc, uint32_t op) /* Store */
{
	uint32_t rn = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = op & THUMB_ADDSUB_RD;
	int32_t offs = (op & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT;
	WRITE8(GetRegister(rn) + offs, GetRegister(rd));
	R15 += 2;
}

void arm7_cpu_device::tg07_1(uint32_t pc, uint32_t op)  /* Load */
{
	uint32_t rn = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = op & THUMB_ADDSUB_RD;
	int32_t offs = (op & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT;
	SetRegister(rd, READ8(GetRegister(rn) + offs));
	R15 += 2;
}

/* Load/Store Halfword */

void arm7_cpu_device::tg08_0(uint32_t pc, uint32_t op) /* Store */
{
	uint32_t imm = (op & THUMB_HALFOP_OFFS) >> THUMB_HALFOP_OFFS_SHIFT;
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	WRITE16(GetRegister(rs) + (imm << 1), GetRegister(rd));
	R15 += 2;
}

void arm7_cpu_device::tg08_1(uint32_t pc, uint32_t op) /* Load */
{
	uint32_t imm = (op & THUMB_HALFOP_OFFS) >> THUMB_HALFOP_OFFS_SHIFT;
	uint32_t rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	uint32_t rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SetRegister(rd, READ16(GetRegister(rs) + (imm << 1)));
	R15 += 2;
}

/* Stack-Relative Load/Store */

void arm7_cpu_device::tg09_0(uint32_t pc, uint32_t op) /* Store */
{
	uint32_t rd = (op & THUMB_STACKOP_RD) >> THUMB_STACKOP_RD_SHIFT;
	int32_t offs = (uint8_t)(op & THUMB_INSN_IMM);
	WRITE32(GetRegister(13) + ((uint32_t)offs << 2), GetRegister(rd));
	R15 += 2;
}

void arm7_cpu_device::tg09_1(uint32_t pc, uint32_t op) /* Load */
{
	uint32_t rd = (op & THUMB_STACKOP_RD) >> THUMB_STACKOP_RD_SHIFT;
	int32_t offs = (uint8_t)(op & THUMB_INSN_IMM);
	uint32_t readword = READ32((GetRegister(13) + ((uint32_t)offs << 2)) & ~3);
	SetRegister(rd, readword);
	R15 += 2;
}

/* Get relative address */

void arm7_cpu_device::tg0a_0(uint32_t pc, uint32_t op)  /* ADD Rd, PC, #nn */
{
	uint32_t rd = (op & THUMB_RELADDR_RD) >> THUMB_RELADDR_RD_SHIFT;
	int32_t offs = (uint8_t)(op & THUMB_INSN_IMM) << 2;
	SetRegister(rd, ((R15 + 4) & ~2) + offs);
	R15 += 2;
}

void arm7_cpu_device::tg0a_1(uint32_t pc, uint32_t op) /* ADD Rd, SP, #nn */
{
	uint32_t rd = (op & THUMB_RELADDR_RD) >> THUMB_RELADDR_RD_SHIFT;
	int32_t offs = (uint8_t)(op & THUMB_INSN_IMM) << 2;
	SetRegister(rd, GetRegister(13) + offs);
	R15 += 2;
}

	/* Stack-Related Opcodes */

void arm7_cpu_device::tg0b_0(uint32_t pc, uint32_t op) /* ADD SP, #imm */
{
	uint32_t addr = (op & THUMB_INSN_IMM);
	addr &= ~THUMB_INSN_IMM_S;
	SetRegister(13, GetRegister(13) + ((op & THUMB_INSN_IMM_S) ? -(addr << 2) : (addr << 2)));
	R15 += 2;
}

void arm7_cpu_device::tg0b_1(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

void arm7_cpu_device::tg0b_2(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

void arm7_cpu_device::tg0b_3(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

void arm7_cpu_device::tg0b_4(uint32_t pc, uint32_t op) /* PUSH {Rlist} */
{
	for (int32_t offs = 7; offs >= 0; offs--)
	{
		if (op & (1 << offs))
		{
			SetRegister(13, GetRegister(13) - 4);
			WRITE32(GetRegister(13), GetRegister(offs));
		}
	}
	R15 += 2;
}

void arm7_cpu_device::tg0b_5(uint32_t pc, uint32_t op) /* PUSH {Rlist}{LR} */
{
	SetRegister(13, GetRegister(13) - 4);
	WRITE32(GetRegister(13), GetRegister(14));
	for (int32_t offs = 7; offs >= 0; offs--)
	{
		if (op & (1 << offs))
		{
			SetRegister(13, GetRegister(13) - 4);
			WRITE32(GetRegister(13), GetRegister(offs));
		}
	}
	R15 += 2;
}

void arm7_cpu_device::tg0b_6(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

void arm7_cpu_device::tg0b_7(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

void arm7_cpu_device::tg0b_8(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

void arm7_cpu_device::tg0b_9(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

void arm7_cpu_device::tg0b_a(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

void arm7_cpu_device::tg0b_b(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

void arm7_cpu_device::tg0b_c(uint32_t pc, uint32_t op) /* POP {Rlist} */
{
	for (int32_t offs = 0; offs < 8; offs++)
	{
		if (op & (1 << offs))
		{
			SetRegister(offs, READ32(GetRegister(13) & ~3));
			SetRegister(13, GetRegister(13) + 4);
		}
	}
	R15 += 2;
}

void arm7_cpu_device::tg0b_d(uint32_t pc, uint32_t op) /* POP {Rlist}{PC} */
{
	for (int32_t offs = 0; offs < 8; offs++)
	{
		if (op & (1 << offs))
		{
			SetRegister(offs, READ32(GetRegister(13) & ~3));
			SetRegister(13, GetRegister(13) + 4);
		}
	}
	uint32_t addr = READ32(GetRegister(13) & ~3);
	if (m_archRev < 5)
	{
		R15 = addr & ~1;
	}
	else
	{
		if (addr & 1)
		{
			addr &= ~1;
		}
		else
		{
			set_cpsr(GET_CPSR & ~T_MASK);
			if (addr & 2)
			{
				addr += 2;
			}
		}

		R15 = addr;
	}
	SetRegister(13, GetRegister(13) + 4);
}

void arm7_cpu_device::tg0b_e(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

void arm7_cpu_device::tg0b_f(uint32_t pc, uint32_t op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

/* Multiple Load/Store */

// "The address should normally be a word aligned quantity and non-word aligned addresses do not affect the instruction."
// "However, the bottom 2 bits of the address will appear on A[1:0] and might be interpreted by the memory system."

// Endrift says LDMIA/STMIA ignore the low 2 bits and GBA Test Suite assumes it.

void arm7_cpu_device::tg0c_0(uint32_t pc, uint32_t op) /* Store */
{
	uint32_t rd = (op & THUMB_MULTLS_BASE) >> THUMB_MULTLS_BASE_SHIFT;
	uint32_t ld_st_address = GetRegister(rd);
	for (int32_t offs = 0; offs < 8; offs++)
	{
		if (op & (1 << offs))
		{
			WRITE32(ld_st_address & ~3, GetRegister(offs));
			ld_st_address += 4;
		}
	}
	SetRegister(rd, ld_st_address);
	R15 += 2;
}

void arm7_cpu_device::tg0c_1(uint32_t pc, uint32_t op) /* Load */
{
	uint32_t rd = (op & THUMB_MULTLS_BASE) >> THUMB_MULTLS_BASE_SHIFT;
	int rd_in_list = op & (1 << rd);
	uint32_t ld_st_address = GetRegister(rd);
	for (int32_t offs = 0; offs < 8; offs++)
	{
		if (op & (1 << offs))
		{
			SetRegister(offs, READ32(ld_st_address & ~3));
			ld_st_address += 4;
		}
	}
	if (!rd_in_list)
	{
		SetRegister(rd, ld_st_address);
	}
	R15 += 2;
}

/* Conditional Branch */

void arm7_cpu_device::tg0d_0(uint32_t pc, uint32_t op) // COND_EQ:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (Z_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

void arm7_cpu_device::tg0d_1(uint32_t pc, uint32_t op) // COND_NE:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (Z_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_2(uint32_t pc, uint32_t op) // COND_CS:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (C_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_3(uint32_t pc, uint32_t op) // COND_CC:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (C_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_4(uint32_t pc, uint32_t op) // COND_MI:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (N_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_5(uint32_t pc, uint32_t op) // COND_PL:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (N_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_6(uint32_t pc, uint32_t op) // COND_VS:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (V_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_7(uint32_t pc, uint32_t op) // COND_VC:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (V_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_8(uint32_t pc, uint32_t op) // COND_HI:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (C_IS_SET(GET_CPSR) && Z_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_9(uint32_t pc, uint32_t op) // COND_LS:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (C_IS_CLEAR(GET_CPSR) || Z_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_a(uint32_t pc, uint32_t op) // COND_GE:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (!(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_b(uint32_t pc, uint32_t op) // COND_LT:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (!(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_c(uint32_t pc, uint32_t op) // COND_GT:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (Z_IS_CLEAR(GET_CPSR) && !(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_d(uint32_t pc, uint32_t op) // COND_LE:
{
	int32_t offs = (int8_t)(op & THUMB_INSN_IMM);
	if (Z_IS_SET(GET_CPSR) || !(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

void arm7_cpu_device::tg0d_e(uint32_t pc, uint32_t op) // COND_AL:
{
	fatalerror("%08x: Undefined Thumb instruction: %04x (ARM9 reserved)\n", pc, op);
}

void arm7_cpu_device::tg0d_f(uint32_t pc, uint32_t op) // COND_NV:   // SWI (this is sort of a "hole" in the opcode encoding)
{
	m_pendingSwi = 1;
	update_irq_state();
	arm7_check_irq_state();
}

/* B #offs */

void arm7_cpu_device::tg0e_0(uint32_t pc, uint32_t op)
{
	int32_t offs = (op & THUMB_BRANCH_OFFS) << 1;
	if (offs & 0x00000800)
	{
		offs |= 0xfffff800;
	}
	R15 += 4 + offs;
}

void arm7_cpu_device::tg0e_1(uint32_t pc, uint32_t op)
{
	/* BLX (LO) */

	uint32_t addr = GetRegister(14);
	addr += (op & THUMB_BLOP_OFFS) << 1;
	addr &= 0xfffffffc;
	SetRegister(14, (R15 + 2) | 1);
	R15 = addr;
	set_cpsr(GET_CPSR & ~T_MASK);
}


void arm7_cpu_device::tg0f_0(uint32_t pc, uint32_t op)
{
	/* BL (HI) */

	uint32_t addr = (op & THUMB_BLOP_OFFS) << 12;
	if (addr & (1 << 22))
	{
		addr |= 0xff800000;
	}
	addr += R15 + 4;
	SetRegister(14, addr);
	R15 += 2;
}

void arm7_cpu_device::tg0f_1(uint32_t pc, uint32_t op) /* BL */
{
	/* BL (LO) */

	uint32_t addr = GetRegister(14) & ~1;
	addr += (op & THUMB_BLOP_OFFS) << 1;
	SetRegister(14, (R15 + 2) | 1);
	R15 = addr;
	//R15 += 2;
}
