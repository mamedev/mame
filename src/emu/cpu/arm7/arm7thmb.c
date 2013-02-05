#include "emu.h"
#include "arm7core.h"
#include "arm7thmb.h"
#include "arm7help.h"

// this is our master dispatch jump table for THUMB mode, representing [(INSN & 0xffc0) >> 6] bits of the 16-bit decoded instruction
arm7thumb_ophandler thumb_handler[0x40*0x10] =
{
// #define THUMB_SHIFT_R       ((UINT16)0x0800)
	tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,
	tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,
// #define THUMB_INSN_ADDSUB   ((UINT16)0x0800)   // #define THUMB_ADDSUB_TYPE   ((UINT16)0x0600)
	tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,
	tg01_10,    tg01_10,    tg01_10,    tg01_10,    tg01_10,    tg01_10,    tg01_10,    tg01_10,    tg01_11,    tg01_11,    tg01_11,    tg01_11,    tg01_11,    tg01_11,    tg01_11,    tg01_11,    tg01_12,    tg01_12,    tg01_12,    tg01_12,    tg01_12,    tg01_12,    tg01_12,    tg01_12,    tg01_13,    tg01_13,    tg01_13,    tg01_13,    tg01_13,    tg01_13,    tg01_13,    tg01_13,
// #define THUMB_INSN_CMP      ((UINT16)0x0800)
	tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,
	tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,
// #define THUMB_INSN_SUB      ((UINT16)0x0800)
	tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,
	tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,
//#define THUMB_GROUP4_TYPE   ((UINT16)0x0c00)  //#define THUMB_ALUOP_TYPE    ((UINT16)0x03c0)  // #define THUMB_HIREG_OP      ((UINT16)0x0300)  // #define THUMB_HIREG_H       ((UINT16)0x00c0)
	tg04_00_00, tg04_00_01, tg04_00_02, tg04_00_03, tg04_00_04, tg04_00_05, tg04_00_06, tg04_00_07, tg04_00_08, tg04_00_09, tg04_00_0a, tg04_00_0b, tg04_00_0c, tg04_00_0d, tg04_00_0e, tg04_00_0f, tg04_01_00, tg04_01_01, tg04_01_02, tg04_01_03, tg04_01_10, tg04_01_11, tg04_01_12, tg04_01_13, tg04_01_20, tg04_01_21, tg04_01_22, tg04_01_23, tg04_01_30, tg04_01_31, tg04_01_32, tg04_01_33,
	tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,
//#define THUMB_GROUP5_TYPE   ((UINT16)0x0e00)
	tg05_0,     tg05_0,     tg05_0,     tg05_0,     tg05_0,     tg05_0,     tg05_0,     tg05_0,     tg05_1,     tg05_1,     tg05_1,     tg05_1,     tg05_1,     tg05_1,     tg05_1,     tg05_1,     tg05_2,     tg05_2,     tg05_2,     tg05_2,     tg05_2,     tg05_2,     tg05_2,     tg05_2,     tg05_3,     tg05_3,     tg05_3,     tg05_3,     tg05_3,     tg05_3,     tg05_3,     tg05_3,
	tg05_4,     tg05_4,     tg05_4,     tg05_4,     tg05_4,     tg05_4,     tg05_4,     tg05_4,     tg05_5,     tg05_5,     tg05_5,     tg05_5,     tg05_5,     tg05_5,     tg05_5,     tg05_5,     tg05_6,     tg05_6,     tg05_6,     tg05_6,     tg05_6,     tg05_6,     tg05_6,     tg05_6,     tg05_7,     tg05_7,     tg05_7,     tg05_7,     tg05_7,     tg05_7,     tg05_7,     tg05_7,
//#define THUMB_LSOP_L        ((UINT16)0x0800)
	tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,
	tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,
//#define THUMB_LSOP_L        ((UINT16)0x0800)
	tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,
	tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,
// #define THUMB_HALFOP_L      ((UINT16)0x0800)
	tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,
	tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,
// #define THUMB_STACKOP_L     ((UINT16)0x0800)
	tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,
	tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,
// #define THUMB_RELADDR_SP    ((UINT16)0x0800)
	tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,
	tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,
// #define THUMB_STACKOP_TYPE  ((UINT16)0x0f00)
	tg0b_0,     tg0b_0,     tg0b_0,     tg0b_0,     tg0b_1,     tg0b_1,     tg0b_1,     tg0b_1,     tg0b_2,     tg0b_2,     tg0b_2,     tg0b_2,     tg0b_3,     tg0b_3,     tg0b_3,     tg0b_3,     tg0b_4,     tg0b_4,     tg0b_4,     tg0b_4,     tg0b_5,     tg0b_5,     tg0b_5,     tg0b_5,     tg0b_6,     tg0b_6,     tg0b_6,     tg0b_6,     tg0b_7,     tg0b_7,     tg0b_7,     tg0b_7,
	tg0b_8,     tg0b_8,     tg0b_8,     tg0b_8,     tg0b_9,     tg0b_9,     tg0b_9,     tg0b_9,     tg0b_a,     tg0b_a,     tg0b_a,     tg0b_a,     tg0b_b,     tg0b_b,     tg0b_b,     tg0b_b,     tg0b_c,     tg0b_c,     tg0b_c,     tg0b_c,     tg0b_d,     tg0b_d,     tg0b_d,     tg0b_d,     tg0b_e,     tg0b_e,     tg0b_e,     tg0b_e,     tg0b_f,     tg0b_f,     tg0b_f,     tg0b_f,
// #define THUMB_MULTLS        ((UINT16)0x0800)
	tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,
	tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,
// #define THUMB_COND_TYPE     ((UINT16)0x0f00)
	tg0d_0,     tg0d_0,     tg0d_0,     tg0d_0,     tg0d_1,     tg0d_1,     tg0d_1,     tg0d_1,     tg0d_2,     tg0d_2,     tg0d_2,     tg0d_2,     tg0d_3,     tg0d_3,     tg0d_3,     tg0d_3,     tg0d_4,     tg0d_4,     tg0d_4,     tg0d_4,     tg0d_5,     tg0d_5,     tg0d_5,     tg0d_5,     tg0d_6,     tg0d_6,     tg0d_6,     tg0d_6,     tg0d_7,     tg0d_7,     tg0d_7,     tg0d_7,
	tg0d_8,     tg0d_8,     tg0d_8,     tg0d_8,     tg0d_9,     tg0d_9,     tg0d_9,     tg0d_9,     tg0d_a,     tg0d_a,     tg0d_a,     tg0d_a,     tg0d_b,     tg0d_b,     tg0d_b,     tg0d_b,     tg0d_c,     tg0d_c,     tg0d_c,     tg0d_c,     tg0d_d,     tg0d_d,     tg0d_d,     tg0d_d,     tg0d_e,     tg0d_e,     tg0d_e,     tg0d_e,     tg0d_f,     tg0d_f,     tg0d_f,     tg0d_f,
// #define THUMB_BLOP_LO       ((UINT16)0x0800)
	tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,
	tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,
// #define THUMB_BLOP_LO       ((UINT16)0x0800)
	tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,
	tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,
};

	/* Shift operations */

const void tg00_0(arm_state *arm, UINT32 pc, UINT32 op) /* Shift left */
{
	UINT32 rs, rd, rrs;
	INT32 offs;

	SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));

	rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rrs = GET_REGISTER(arm, rs);
	offs = (op & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
	if (offs != 0)
	{
		SET_REGISTER(arm, rd, rrs << offs);
		if (rrs & (1 << (31 - (offs - 1))))
		{
			SET_CPSR(GET_CPSR | C_MASK);
		}
		else
		{
			SET_CPSR(GET_CPSR & ~C_MASK);
		}
	}
	else
	{
		SET_REGISTER(arm, rd, rrs);
	}
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
	R15 += 2;
}

const void tg00_1(arm_state *arm, UINT32 pc, UINT32 op) /* Shift right */
{
	UINT32 rs, rd, rrs;
	INT32 offs;

	rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rrs = GET_REGISTER(arm, rs);
	offs = (op & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
	if (offs != 0)
	{
		SET_REGISTER(arm, rd, rrs >> offs);
		if (rrs & (1 << (offs - 1)))
		{
			SET_CPSR(GET_CPSR | C_MASK);
		}
		else
		{
			SET_CPSR(GET_CPSR & ~C_MASK);
		}
	}
	else
	{
		SET_REGISTER(arm, rd, 0);
		if (rrs & 0x80000000)
		{
			SET_CPSR(GET_CPSR | C_MASK);
		}
		else
		{
			SET_CPSR(GET_CPSR & ~C_MASK);
		}
	}
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
	R15 += 2;
}

	/* Arithmetic */

const void tg01_0(arm_state *arm, UINT32 pc, UINT32 op)
{
	UINT32 rs, rd, rrs;
	INT32 offs;
	/* ASR.. */
	//if (op & THUMB_SHIFT_R) /* Shift right */
	{
		rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
		rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
		rrs = GET_REGISTER(arm, rs);
		offs = (op & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
		if (offs == 0)
		{
			offs = 32;
		}
		if (offs >= 32)
		{
			if (rrs >> 31)
			{
				SET_CPSR(GET_CPSR | C_MASK);
			}
			else
			{
				SET_CPSR(GET_CPSR & ~C_MASK);
			}
			SET_REGISTER(arm, rd, (rrs & 0x80000000) ? 0xFFFFFFFF : 0x00000000);
		}
		else
		{
			if ((rrs >> (offs - 1)) & 1)
			{
				SET_CPSR(GET_CPSR | C_MASK);
			}
			else
			{
				SET_CPSR(GET_CPSR & ~C_MASK);
			}
			SET_REGISTER(arm, rd,
							(rrs & 0x80000000)
							? ((0xFFFFFFFF << (32 - offs)) | (rrs >> offs))
							: (rrs >> offs));
		}
		SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));
		SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
		R15 += 2;
	}
}

const void tg01_10(arm_state *arm, UINT32 pc, UINT32 op)  /* ADD Rd, Rs, Rn */
{
	UINT32 rn = GET_REGISTER(arm, (op & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT);
	UINT32 rs = GET_REGISTER(arm, (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(arm, rd, rs + rn);
	HandleThumbALUAddFlags(GET_REGISTER(arm, rd), rs, rn);

}

const void tg01_11(arm_state *arm, UINT32 pc, UINT32 op) /* SUB Rd, Rs, Rn */
{
	UINT32 rn = GET_REGISTER(arm, (op & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT);
	UINT32 rs = GET_REGISTER(arm, (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(arm, rd, rs - rn);
	HandleThumbALUSubFlags(GET_REGISTER(arm, rd), rs, rn);

}

const void tg01_12(arm_state *arm, UINT32 pc, UINT32 op) /* ADD Rd, Rs, #imm */
{
	UINT32 imm = (op & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT;
	UINT32 rs = GET_REGISTER(arm, (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(arm, rd, rs + imm);
	HandleThumbALUAddFlags(GET_REGISTER(arm, rd), rs, imm);

}

const void tg01_13(arm_state *arm, UINT32 pc, UINT32 op) /* SUB Rd, Rs, #imm */
{
	UINT32 imm = (op & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT;
	UINT32 rs = GET_REGISTER(arm, (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(arm, rd, rs - imm);
	HandleThumbALUSubFlags(GET_REGISTER(arm, rd), rs,imm);

}

	/* CMP / MOV */

const void tg02_0(arm_state *arm, UINT32 pc, UINT32 op)
{
	UINT32 rd = (op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT;
	UINT32 op2 = (op & THUMB_INSN_IMM);
	SET_REGISTER(arm, rd, op2);
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
	R15 += 2;
}

const void tg02_1(arm_state *arm, UINT32 pc, UINT32 op)
{
	UINT32 rn = GET_REGISTER(arm, (op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
	UINT32 op2 = op & THUMB_INSN_IMM;
	UINT32 rd = rn - op2;
	HandleThumbALUSubFlags(rd, rn, op2);
}

	/* ADD/SUB immediate */

const void tg03_0(arm_state *arm, UINT32 pc, UINT32 op) /* ADD Rd, #Offset8 */
{
	UINT32 rn = GET_REGISTER(arm, (op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
	UINT32 op2 = op & THUMB_INSN_IMM;
	UINT32 rd = rn + op2;
	SET_REGISTER(arm, (op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, rd);
	HandleThumbALUAddFlags(rd, rn, op2);
}

const void tg03_1(arm_state *arm, UINT32 pc, UINT32 op) /* SUB Rd, #Offset8 */
{
	UINT32 rn = GET_REGISTER(arm, (op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
	UINT32 op2 = op & THUMB_INSN_IMM;
	UINT32 rd = rn - op2;
	SET_REGISTER(arm, (op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, rd);
	HandleThumbALUSubFlags(rd, rn, op2);
}

	/* Rd & Rm instructions */

const void tg04_00_00(arm_state *arm, UINT32 pc, UINT32 op) /* AND Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(arm, rd, GET_REGISTER(arm, rd) & GET_REGISTER(arm, rs));
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
	R15 += 2;
}

const void tg04_00_01(arm_state *arm, UINT32 pc, UINT32 op) /* EOR Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(arm, rd, GET_REGISTER(arm, rd) ^ GET_REGISTER(arm, rs));
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
	R15 += 2;
}

const void tg04_00_02(arm_state *arm, UINT32 pc, UINT32 op) /* LSL Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	UINT32 rrd = GET_REGISTER(arm, rd);
	INT32 offs = GET_REGISTER(arm, rs) & 0x000000ff;
	if (offs > 0)
	{
		if (offs < 32)
		{
			SET_REGISTER(arm, rd, rrd << offs);
			if (rrd & (1 << (31 - (offs - 1))))
			{
				SET_CPSR(GET_CPSR | C_MASK);
			}
			else
			{
				SET_CPSR(GET_CPSR & ~C_MASK);
			}
		}
		else if (offs == 32)
		{
			SET_REGISTER(arm, rd, 0);
			if (rrd & 1)
			{
				SET_CPSR(GET_CPSR | C_MASK);
			}
			else
			{
				SET_CPSR(GET_CPSR & ~C_MASK);
			}
		}
		else
		{
			SET_REGISTER(arm, rd, 0);
			SET_CPSR(GET_CPSR & ~C_MASK);
		}
	}
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
	R15 += 2;
}

const void tg04_00_03(arm_state *arm, UINT32 pc, UINT32 op) /* LSR Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	UINT32 rrd = GET_REGISTER(arm, rd);
	INT32 offs = GET_REGISTER(arm, rs) & 0x000000ff;
	if (offs >  0)
	{
		if (offs < 32)
		{
			SET_REGISTER(arm, rd, rrd >> offs);
			if (rrd & (1 << (offs - 1)))
			{
				SET_CPSR(GET_CPSR | C_MASK);
			}
			else
			{
				SET_CPSR(GET_CPSR & ~C_MASK);
			}
		}
		else if (offs == 32)
		{
			SET_REGISTER(arm, rd, 0);
			if (rrd & 0x80000000)
			{
				SET_CPSR(GET_CPSR | C_MASK);
			}
			else
			{
				SET_CPSR(GET_CPSR & ~C_MASK);
			}
		}
		else
		{
			SET_REGISTER(arm, rd, 0);
			SET_CPSR(GET_CPSR & ~C_MASK);
		}
	}
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
	R15 += 2;
}

const void tg04_00_04(arm_state *arm, UINT32 pc, UINT32 op) /* ASR Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	UINT32 rrs = GET_REGISTER(arm, rs)&0xff;
	UINT32 rrd = GET_REGISTER(arm, rd);
	if (rrs != 0)
	{
		if (rrs >= 32)
		{
			if (rrd >> 31)
			{
				SET_CPSR(GET_CPSR | C_MASK);
			}
			else
			{
				SET_CPSR(GET_CPSR & ~C_MASK);
			}
			SET_REGISTER(arm, rd, (GET_REGISTER(arm, rd) & 0x80000000) ? 0xFFFFFFFF : 0x00000000);
		}
		else
		{
			if ((rrd >> (rrs-1)) & 1)
			{
				SET_CPSR(GET_CPSR | C_MASK);
			}
			else
			{
				SET_CPSR(GET_CPSR & ~C_MASK);
			}
			SET_REGISTER(arm, rd, (rrd & 0x80000000)
							? ((0xFFFFFFFF << (32 - rrs)) | (rrd >> rrs))
							: (rrd >> rrs));
		}
	}
	SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
	R15 += 2;
}

const void tg04_00_05(arm_state *arm, UINT32 pc, UINT32 op) /* ADC Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	UINT32 op2 = (GET_CPSR & C_MASK) ? 1 : 0;
	UINT32 rn = GET_REGISTER(arm, rd) + GET_REGISTER(arm, rs) + op2;
	HandleThumbALUAddFlags(rn, GET_REGISTER(arm, rd), (GET_REGISTER(arm, rs))); // ?
	SET_REGISTER(arm, rd, rn);
}

const void tg04_00_06(arm_state *arm, UINT32 pc, UINT32 op)  /* SBC Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	UINT32 op2 = (GET_CPSR & C_MASK) ? 0 : 1;
	UINT32 rn = GET_REGISTER(arm, rd) - GET_REGISTER(arm, rs) - op2;
	HandleThumbALUSubFlags(rn, GET_REGISTER(arm, rd), (GET_REGISTER(arm, rs))); //?
	SET_REGISTER(arm, rd, rn);
}

const void tg04_00_07(arm_state *arm, UINT32 pc, UINT32 op) /* ROR Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	UINT32 rrd = GET_REGISTER(arm, rd);
	UINT32 imm = GET_REGISTER(arm, rs) & 0x0000001f;
	SET_REGISTER(arm, rd, (rrd >> imm) | (rrd << (32 - imm)));
	if (rrd & (1 << (imm - 1)))
	{
		SET_CPSR(GET_CPSR | C_MASK);
	}
	else
	{
		SET_CPSR(GET_CPSR & ~C_MASK);
	}
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
	R15 += 2;
}

const void tg04_00_08(arm_state *arm, UINT32 pc, UINT32 op) /* TST Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd) & GET_REGISTER(arm, rs)));
	R15 += 2;
}

const void tg04_00_09(arm_state *arm, UINT32 pc, UINT32 op) /* NEG Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	UINT32 rrs = GET_REGISTER(arm, rs);
	SET_REGISTER(arm, rd, 0 - rrs);
	HandleThumbALUSubFlags(GET_REGISTER(arm, rd), 0, rrs);
}

const void tg04_00_0a(arm_state *arm, UINT32 pc, UINT32 op) /* CMP Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	UINT32 rn = GET_REGISTER(arm, rd) - GET_REGISTER(arm, rs);
	HandleThumbALUSubFlags(rn, GET_REGISTER(arm, rd), GET_REGISTER(arm, rs));
}

const void tg04_00_0b(arm_state *arm, UINT32 pc, UINT32 op) /* CMN Rd, Rs - check flags, add dasm */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	UINT32 rn = GET_REGISTER(arm, rd) + GET_REGISTER(arm, rs);
	HandleThumbALUAddFlags(rn, GET_REGISTER(arm, rd), GET_REGISTER(arm, rs));
}

const void tg04_00_0c(arm_state *arm, UINT32 pc, UINT32 op) /* ORR Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(arm, rd, GET_REGISTER(arm, rd) | GET_REGISTER(arm, rs));
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
	R15 += 2;
}

const void tg04_00_0d(arm_state *arm, UINT32 pc, UINT32 op) /* MUL Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	UINT32 rn = GET_REGISTER(arm, rd) * GET_REGISTER(arm, rs);
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_REGISTER(arm, rd, rn);
	SET_CPSR(GET_CPSR | HandleALUNZFlags(rn));
	R15 += 2;
}

const void tg04_00_0e(arm_state *arm, UINT32 pc, UINT32 op) /* BIC Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(arm, rd, GET_REGISTER(arm, rd) & (~GET_REGISTER(arm, rs)));
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
	R15 += 2;
}

const void tg04_00_0f(arm_state *arm, UINT32 pc, UINT32 op) /* MVN Rd, Rs */
{
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	UINT32 op2 = GET_REGISTER(arm, rs);
	SET_REGISTER(arm, rd, ~op2);
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(arm, rd)));
	R15 += 2;
}

/* ADD Rd, Rs group */

const void tg04_01_00(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: G4-1-0 Undefined Thumb instruction: %04x %x\n", pc, op, (op & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT);
}

const void tg04_01_01(arm_state *arm, UINT32 pc, UINT32 op) /* ADD Rd, HRs */
{
	UINT32 rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	UINT32 rd = op & THUMB_HIREG_RD;
	SET_REGISTER(arm, rd, GET_REGISTER(arm, rd) + GET_REGISTER(arm, rs+8));
	// emulate the effects of pre-fetch
	if (rs == 7)
	{
		SET_REGISTER(arm, rd, GET_REGISTER(arm, rd) + 4);
	}

	R15 += 2;
}

const void tg04_01_02(arm_state *arm, UINT32 pc, UINT32 op) /* ADD HRd, Rs */
{
	UINT32 rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	UINT32 rd = op & THUMB_HIREG_RD;
	SET_REGISTER(arm, rd+8, GET_REGISTER(arm, rd+8) + GET_REGISTER(arm, rs));
	if (rd == 7)
	{
		R15 += 2;
	}

	R15 += 2;
}

const void tg04_01_03(arm_state *arm, UINT32 pc, UINT32 op) /* Add HRd, HRs */
{
	UINT32 rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	UINT32 rd = op & THUMB_HIREG_RD;
	SET_REGISTER(arm, rd+8, GET_REGISTER(arm, rd+8) + GET_REGISTER(arm, rs+8));
	// emulate the effects of pre-fetch
	if (rs == 7)
	{
		SET_REGISTER(arm, rd+8, GET_REGISTER(arm, rd+8) + 4);
	}
	if (rd == 7)
	{
		R15 += 2;
	}

	R15 += 2;
}

const void tg04_01_10(arm_state *arm, UINT32 pc, UINT32 op)  /* CMP Rd, Rs */
{
	UINT32 rs = GET_REGISTER(arm, ((op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT));
	UINT32 rd = GET_REGISTER(arm, op & THUMB_HIREG_RD);
	UINT32 rn = rd - rs;
	HandleThumbALUSubFlags(rn, rd, rs);
}

const void tg04_01_11(arm_state *arm, UINT32 pc, UINT32 op) /* CMP Rd, Hs */
{
	UINT32 rs = GET_REGISTER(arm, ((op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
	UINT32 rd = GET_REGISTER(arm, op & THUMB_HIREG_RD);
	UINT32 rn = rd - rs;
	HandleThumbALUSubFlags(rn, rd, rs);
}

const void tg04_01_12(arm_state *arm, UINT32 pc, UINT32 op) /* CMP Hd, Rs */
{
	UINT32 rs = GET_REGISTER(arm, ((op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT));
	UINT32 rd = GET_REGISTER(arm, (op & THUMB_HIREG_RD) + 8);
	UINT32 rn = rd - rs;
	HandleThumbALUSubFlags(rn, rd, rs);
}

const void tg04_01_13(arm_state *arm, UINT32 pc, UINT32 op) /* CMP Hd, Hs */
{
	UINT32 rs = GET_REGISTER(arm, ((op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
	UINT32 rd = GET_REGISTER(arm, (op & THUMB_HIREG_RD) + 8);
	UINT32 rn = rd - rs;
	HandleThumbALUSubFlags(rn, rd, rs);
}

/* MOV group */

// "The action of H1 = 0, H2 = 0 for Op = 00 (ADD), Op = 01 (CMP) and Op = 10 (MOV) is undefined, and should not be used."
const void tg04_01_20(arm_state *arm, UINT32 pc, UINT32 op) /* MOV Rd, Rs (undefined) */
{
	UINT32 rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	UINT32 rd = op & THUMB_HIREG_RD;
	SET_REGISTER(arm, rd, GET_REGISTER(arm, rs));
	R15 += 2;
}

const void tg04_01_21(arm_state *arm, UINT32 pc, UINT32 op) /* MOV Rd, Hs */
{
	UINT32 rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	UINT32 rd = op & THUMB_HIREG_RD;
	SET_REGISTER(arm, rd, GET_REGISTER(arm, rs + 8));
	if (rs == 7)
	{
		SET_REGISTER(arm, rd, GET_REGISTER(arm, rd) + 4);
	}
	R15 += 2;
}

const void tg04_01_22(arm_state *arm, UINT32 pc, UINT32 op) /* MOV Hd, Rs */
{
	UINT32 rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	UINT32 rd = op & THUMB_HIREG_RD;
	SET_REGISTER(arm, rd + 8, GET_REGISTER(arm, rs));
	if (rd != 7)
	{
		R15 += 2;
	}
	else
	{
		R15 &= ~1;
	}
}

const void tg04_01_23(arm_state *arm, UINT32 pc, UINT32 op) /* MOV Hd, Hs */
{
	UINT32 rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	UINT32 rd = op & THUMB_HIREG_RD;
	if (rs == 7)
	{
		SET_REGISTER(arm, rd + 8, GET_REGISTER(arm, rs+8)+4);
	}
	else
	{
		SET_REGISTER(arm, rd + 8, GET_REGISTER(arm, rs+8));
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

const void tg04_01_30(arm_state *arm, UINT32 pc, UINT32 op)
{
	UINT32 rd = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	UINT32 addr = GET_REGISTER(arm, rd);
	if (addr & 1)
	{
		addr &= ~1;
	}
	else
	{
		SET_CPSR(GET_CPSR & ~T_MASK);
		if (addr & 2)
		{
			addr += 2;
		}
	}
	R15 = addr;
}

const void tg04_01_31(arm_state *arm, UINT32 pc, UINT32 op)
{
	UINT32 rs = (op & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	UINT32 addr = GET_REGISTER(arm, rs+8);
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
		SET_CPSR(GET_CPSR & ~T_MASK);
		if (addr & 2)
		{
			addr += 2;
		}
	}
	R15 = addr;
}

const void tg04_01_32(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: G4-3 Undefined Thumb instruction: %04x\n", pc, op);
}

const void tg04_01_33(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: G4-3 Undefined Thumb instruction: %04x\n", pc, op);
}

const void tg04_0203(arm_state *arm, UINT32 pc, UINT32 op)
{
	UINT32 readword = READ32((R15 & ~2) + 4 + ((op & THUMB_INSN_IMM) << 2));
	SET_REGISTER(arm, (op & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, readword);
	R15 += 2;
}

/* LDR* STR* group */

const void tg05_0(arm_state *arm, UINT32 pc, UINT32 op)  /* STR Rd, [Rn, Rm] */
{
	UINT32 rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	UINT32 rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	UINT32 rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	UINT32 addr = GET_REGISTER(arm, rn) + GET_REGISTER(arm, rm);
	WRITE32(addr, GET_REGISTER(arm, rd));
	R15 += 2;
}

const void tg05_1(arm_state *arm, UINT32 pc, UINT32 op)  /* STRH Rd, [Rn, Rm] */
{
	UINT32 rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	UINT32 rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	UINT32 rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	UINT32 addr = GET_REGISTER(arm, rn) + GET_REGISTER(arm, rm);
	WRITE16(addr, GET_REGISTER(arm, rd));
	R15 += 2;
}

const void tg05_2(arm_state *arm, UINT32 pc, UINT32 op)  /* STRB Rd, [Rn, Rm] */
{
	UINT32 rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	UINT32 rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	UINT32 rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	UINT32 addr = GET_REGISTER(arm, rn) + GET_REGISTER(arm, rm);
	WRITE8(addr, GET_REGISTER(arm, rd));
	R15 += 2;
}

const void tg05_3(arm_state *arm, UINT32 pc, UINT32 op)  /* LDSB Rd, [Rn, Rm] todo, add dasm */
{
	UINT32 rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	UINT32 rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	UINT32 rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	UINT32 addr = GET_REGISTER(arm, rn) + GET_REGISTER(arm, rm);
	UINT32 op2 = READ8(addr);
	if (op2 & 0x00000080)
	{
		op2 |= 0xffffff00;
	}
	SET_REGISTER(arm, rd, op2);
	R15 += 2;
}

const void tg05_4(arm_state *arm, UINT32 pc, UINT32 op)  /* LDR Rd, [Rn, Rm] */
{
	UINT32 rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	UINT32 rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	UINT32 rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	UINT32 addr = GET_REGISTER(arm, rn) + GET_REGISTER(arm, rm);
	UINT32 op2 = READ32(addr);
	SET_REGISTER(arm, rd, op2);
	R15 += 2;
}

const void tg05_5(arm_state *arm, UINT32 pc, UINT32 op)  /* LDRH Rd, [Rn, Rm] */
{
	UINT32 rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	UINT32 rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	UINT32 rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	UINT32 addr = GET_REGISTER(arm, rn) + GET_REGISTER(arm, rm);
	UINT32 op2 = READ16(addr);
	SET_REGISTER(arm, rd, op2);
	R15 += 2;
}

const void tg05_6(arm_state *arm, UINT32 pc, UINT32 op)  /* LDRB Rd, [Rn, Rm] */
{
	UINT32 rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	UINT32 rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	UINT32 rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	UINT32 addr = GET_REGISTER(arm, rn) + GET_REGISTER(arm, rm);
	UINT32 op2 = READ8(addr);
	SET_REGISTER(arm, rd, op2);
	R15 += 2;
}

const void tg05_7(arm_state *arm, UINT32 pc, UINT32 op)  /* LDSH Rd, [Rn, Rm] */
{
	UINT32 rm = (op & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	UINT32 rn = (op & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	UINT32 rd = (op & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	UINT32 addr = GET_REGISTER(arm, rn) + GET_REGISTER(arm, rm);
	UINT32 op2 = READ16(addr);
	if (op2 & 0x00008000)
	{
		op2 |= 0xffff0000;
	}
	SET_REGISTER(arm, rd, op2);
	R15 += 2;
}

	/* Word Store w/ Immediate Offset */

const void tg06_0(arm_state *arm, UINT32 pc, UINT32 op) /* Store */
{
	UINT32 rn = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = op & THUMB_ADDSUB_RD;
	INT32 offs = ((op & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT) << 2;
	WRITE32(GET_REGISTER(arm, rn) + offs, GET_REGISTER(arm, rd));
	R15 += 2;
}

const void tg06_1(arm_state *arm, UINT32 pc, UINT32 op) /* Load */
{
	UINT32 rn = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = op & THUMB_ADDSUB_RD;
	INT32 offs = ((op & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT) << 2;
	SET_REGISTER(arm, rd, READ32(GET_REGISTER(arm, rn) + offs)); // fix
	R15 += 2;
}

/* Byte Store w/ Immeidate Offset */

const void tg07_0(arm_state *arm, UINT32 pc, UINT32 op) /* Store */
{
	UINT32 rn = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = op & THUMB_ADDSUB_RD;
	INT32 offs = (op & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT;
	WRITE8(GET_REGISTER(arm, rn) + offs, GET_REGISTER(arm, rd));
	R15 += 2;
}

const void tg07_1(arm_state *arm, UINT32 pc, UINT32 op)  /* Load */
{
	UINT32 rn = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = op & THUMB_ADDSUB_RD;
	INT32 offs = (op & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT;
	SET_REGISTER(arm, rd, READ8(GET_REGISTER(arm, rn) + offs));
	R15 += 2;
}

/* Load/Store Halfword */

const void tg08_0(arm_state *arm, UINT32 pc, UINT32 op) /* Store */
{
	UINT32 imm = (op & THUMB_HALFOP_OFFS) >> THUMB_HALFOP_OFFS_SHIFT;
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	WRITE16(GET_REGISTER(arm, rs) + (imm << 1), GET_REGISTER(arm, rd));
	R15 += 2;
}

const void tg08_1(arm_state *arm, UINT32 pc, UINT32 op) /* Load */
{
	UINT32 imm = (op & THUMB_HALFOP_OFFS) >> THUMB_HALFOP_OFFS_SHIFT;
	UINT32 rs = (op & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	UINT32 rd = (op & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(arm, rd, READ16(GET_REGISTER(arm, rs) + (imm << 1)));
	R15 += 2;
}

/* Stack-Relative Load/Store */

const void tg09_0(arm_state *arm, UINT32 pc, UINT32 op) /* Store */
{
	UINT32 rd = (op & THUMB_STACKOP_RD) >> THUMB_STACKOP_RD_SHIFT;
	INT32 offs = (UINT8)(op & THUMB_INSN_IMM);
	WRITE32(GET_REGISTER(arm, 13) + ((UINT32)offs << 2), GET_REGISTER(arm, rd));
	R15 += 2;
}

const void tg09_1(arm_state *arm, UINT32 pc, UINT32 op) /* Load */
{
	UINT32 rd = (op & THUMB_STACKOP_RD) >> THUMB_STACKOP_RD_SHIFT;
	INT32 offs = (UINT8)(op & THUMB_INSN_IMM);
	UINT32 readword = READ32(GET_REGISTER(arm, 13) + ((UINT32)offs << 2));
	SET_REGISTER(arm, rd, readword);
	R15 += 2;
}

/* Get relative address */

const void tg0a_0(arm_state *arm, UINT32 pc, UINT32 op)  /* ADD Rd, PC, #nn */
{
	UINT32 rd = (op & THUMB_RELADDR_RD) >> THUMB_RELADDR_RD_SHIFT;
	INT32 offs = (UINT8)(op & THUMB_INSN_IMM) << 2;
	SET_REGISTER(arm, rd, ((R15 + 4) & ~2) + offs);
	R15 += 2;
}

const void tg0a_1(arm_state *arm, UINT32 pc, UINT32 op) /* ADD Rd, SP, #nn */
{
	UINT32 rd = (op & THUMB_RELADDR_RD) >> THUMB_RELADDR_RD_SHIFT;
	INT32 offs = (UINT8)(op & THUMB_INSN_IMM) << 2;
	SET_REGISTER(arm, rd, GET_REGISTER(arm, 13) + offs);
	R15 += 2;
}

	/* Stack-Related Opcodes */

const void tg0b_0(arm_state *arm, UINT32 pc, UINT32 op) /* ADD SP, #imm */
{
	UINT32 addr = (op & THUMB_INSN_IMM);
	addr &= ~THUMB_INSN_IMM_S;
	SET_REGISTER(arm, 13, GET_REGISTER(arm, 13) + ((op & THUMB_INSN_IMM_S) ? -(addr << 2) : (addr << 2)));
	R15 += 2;
}

const void tg0b_1(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

const void tg0b_2(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

const void tg0b_3(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

const void tg0b_4(arm_state *arm, UINT32 pc, UINT32 op) /* PUSH {Rlist} */
{
	for (INT32 offs = 7; offs >= 0; offs--)
	{
		if (op & (1 << offs))
		{
			SET_REGISTER(arm, 13, GET_REGISTER(arm, 13) - 4);
			WRITE32(GET_REGISTER(arm, 13), GET_REGISTER(arm, offs));
		}
	}
	R15 += 2;
}

const void tg0b_5(arm_state *arm, UINT32 pc, UINT32 op) /* PUSH {Rlist}{LR} */
{
	SET_REGISTER(arm, 13, GET_REGISTER(arm, 13) - 4);
	WRITE32(GET_REGISTER(arm, 13), GET_REGISTER(arm, 14));
	for (INT32 offs = 7; offs >= 0; offs--)
	{
		if (op & (1 << offs))
		{
			SET_REGISTER(arm, 13, GET_REGISTER(arm, 13) - 4);
			WRITE32(GET_REGISTER(arm, 13), GET_REGISTER(arm, offs));
		}
	}
	R15 += 2;
}

const void tg0b_6(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

const void tg0b_7(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

const void tg0b_8(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

const void tg0b_9(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

const void tg0b_a(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

const void tg0b_b(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

const void tg0b_c(arm_state *arm, UINT32 pc, UINT32 op) /* POP {Rlist} */
{
	for (INT32 offs = 0; offs < 8; offs++)
	{
		if (op & (1 << offs))
		{
			SET_REGISTER(arm, offs, READ32(GET_REGISTER(arm, 13)));
			SET_REGISTER(arm, 13, GET_REGISTER(arm, 13) + 4);
		}
	}
	R15 += 2;
}

const void tg0b_d(arm_state *arm, UINT32 pc, UINT32 op) /* POP {Rlist}{PC} */
{
	for (INT32 offs = 0; offs < 8; offs++)
	{
		if (op & (1 << offs))
		{
			SET_REGISTER(arm, offs, READ32(GET_REGISTER(arm, 13)));
			SET_REGISTER(arm, 13, GET_REGISTER(arm, 13) + 4);
		}
	}
	UINT32 addr = READ32(GET_REGISTER(arm, 13));
	if (arm->archRev < 5)
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
			SET_CPSR(GET_CPSR & ~T_MASK);
			if (addr & 2)
			{
				addr += 2;
			}
		}

		R15 = addr;
	}
	SET_REGISTER(arm, 13, GET_REGISTER(arm, 13) + 4);
}

const void tg0b_e(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

const void tg0b_f(arm_state *arm, UINT32 pc, UINT32 op)
{
	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, op);
}

/* Multiple Load/Store */

// "The address should normally be a word aligned quantity and non-word aligned addresses do not affect the instruction."
// "However, the bottom 2 bits of the address will appear on A[1:0] and might be interpreted by the memory system."

// GBA "BB Ball" performs an unaligned read with A[1:0] = 2 and expects A[1] not to be ignored [BP 800B90A,(R4&3)!=0]
// GBA "Gadget Racers" performs an unaligned read with A[1:0] = 1 and expects A[0] to be ignored [BP B72,(R0&3)!=0]

const void tg0c_0(arm_state *arm, UINT32 pc, UINT32 op) /* Store */
{
	UINT32 rd = (op & THUMB_MULTLS_BASE) >> THUMB_MULTLS_BASE_SHIFT;
	UINT32 ld_st_address = GET_REGISTER(arm, rd);
	for (INT32 offs = 0; offs < 8; offs++)
	{
		if (op & (1 << offs))
		{
			WRITE32(ld_st_address & ~3, GET_REGISTER(arm, offs));
			ld_st_address += 4;
		}
	}
	SET_REGISTER(arm, rd, ld_st_address);
	R15 += 2;
}

const void tg0c_1(arm_state *arm, UINT32 pc, UINT32 op) /* Load */
{
	UINT32 rd = (op & THUMB_MULTLS_BASE) >> THUMB_MULTLS_BASE_SHIFT;
	int rd_in_list = op & (1 << rd);
	UINT32 ld_st_address = GET_REGISTER(arm, rd);
	for (INT32 offs = 0; offs < 8; offs++)
	{
		if (op & (1 << offs))
		{
			SET_REGISTER(arm, offs, READ32(ld_st_address & ~1));
			ld_st_address += 4;
		}
	}
	if (!rd_in_list)
	{
		SET_REGISTER(arm, rd, ld_st_address);
	}
	R15 += 2;
}

/* Conditional Branch */

const void tg0d_0(arm_state *arm, UINT32 pc, UINT32 op) // COND_EQ:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (Z_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_1(arm_state *arm, UINT32 pc, UINT32 op) // COND_NE:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (Z_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_2(arm_state *arm, UINT32 pc, UINT32 op) // COND_CS:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (C_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_3(arm_state *arm, UINT32 pc, UINT32 op) // COND_CC:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (C_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_4(arm_state *arm, UINT32 pc, UINT32 op) // COND_MI:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (N_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_5(arm_state *arm, UINT32 pc, UINT32 op) // COND_PL:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (N_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_6(arm_state *arm, UINT32 pc, UINT32 op) // COND_VS:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (V_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_7(arm_state *arm, UINT32 pc, UINT32 op) // COND_VC:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (V_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_8(arm_state *arm, UINT32 pc, UINT32 op) // COND_HI:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (C_IS_SET(GET_CPSR) && Z_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_9(arm_state *arm, UINT32 pc, UINT32 op) // COND_LS:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (C_IS_CLEAR(GET_CPSR) || Z_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_a(arm_state *arm, UINT32 pc, UINT32 op) // COND_GE:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (!(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_b(arm_state *arm, UINT32 pc, UINT32 op) // COND_LT:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (!(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_c(arm_state *arm, UINT32 pc, UINT32 op) // COND_GT:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (Z_IS_CLEAR(GET_CPSR) && !(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_d(arm_state *arm, UINT32 pc, UINT32 op) // COND_LE:
{
	INT32 offs = (INT8)(op & THUMB_INSN_IMM);
	if (Z_IS_SET(GET_CPSR) || !(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}
}

const void tg0d_e(arm_state *arm, UINT32 pc, UINT32 op) // COND_AL:
{
	fatalerror("%08x: Undefined Thumb instruction: %04x (ARM9 reserved)\n", pc, op);
}

const void tg0d_f(arm_state *arm, UINT32 pc, UINT32 op) // COND_NV:   // SWI (this is sort of a "hole" in the opcode encoding)
{
	arm->pendingSwi = 1;
	ARM7_CHECKIRQ;
}

/* B #offs */

const void tg0e_0(arm_state *arm, UINT32 pc, UINT32 op)
{
	INT32 offs = (op & THUMB_BRANCH_OFFS) << 1;
	if (offs & 0x00000800)
	{
		offs |= 0xfffff800;
	}
	R15 += 4 + offs;
}

const void tg0e_1(arm_state *arm, UINT32 pc, UINT32 op)
{
	UINT32 addr = GET_REGISTER(arm, 14);
	addr += (op & THUMB_BLOP_OFFS) << 1;
	addr &= 0xfffffffc;
	SET_REGISTER(arm, 14, (R15 + 4) | 1);
	R15 = addr;
}

	/* BL */

const void tg0f_0(arm_state *arm, UINT32 pc, UINT32 op)
{
	UINT32 addr = (op & THUMB_BLOP_OFFS) << 12;
	if (addr & (1 << 22))
	{
		addr |= 0xff800000;
	}
	addr += R15 + 4;
	SET_REGISTER(arm, 14, addr);
	R15 += 2;
}

const void tg0f_1(arm_state *arm, UINT32 pc, UINT32 op) /* BL */
{
	UINT32 addr = GET_REGISTER(arm, 14) & ~1;
	addr += (op & THUMB_BLOP_OFFS) << 1;
	SET_REGISTER(arm, 14, (R15 + 2) | 1);
	R15 = addr;
	//R15 += 2;
}
