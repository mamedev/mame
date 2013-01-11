#include "emu.h"
#include "arm7core.h"
#include "arm7thmb.h"
#include "arm7help.h"

// this is our master dispatch jump table for THUMB mode, representing [(insn & 0xffc0) >> 6] bits of the 16-bit decoded instruction
arm7thumb_ophandler thumb_handler[0x40*0x10] =
{
// #define THUMB_SHIFT_R       ((UINT16)0x0800)
	tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_0,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,     tg00_1,
// #define THUMB_INSN_ADDSUB   ((UINT16)0x0800)   // #define THUMB_ADDSUB_TYPE   ((UINT16)0x0600)
	tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_0,     tg01_10,    tg01_10,    tg01_10,    tg01_10,    tg01_10,    tg01_10,    tg01_10,    tg01_10,    tg01_11,    tg01_11,    tg01_11,    tg01_11,    tg01_11,    tg01_11,    tg01_11,    tg01_11,    tg01_12,    tg01_12,    tg01_12,    tg01_12,    tg01_12,    tg01_12,    tg01_12,    tg01_12,    tg01_13,    tg01_13,    tg01_13,    tg01_13,    tg01_13,    tg01_13,    tg01_13,    tg01_13,
// #define THUMB_INSN_CMP      ((UINT16)0x0800)
	tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_0,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,     tg02_1,
// #define THUMB_INSN_SUB      ((UINT16)0x0800)
	tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_0,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,     tg03_1,
//#define THUMB_GROUP4_TYPE   ((UINT16)0x0c00)  //#define THUMB_ALUOP_TYPE    ((UINT16)0x03c0)  // #define THUMB_HIREG_OP      ((UINT16)0x0300)  // #define THUMB_HIREG_H       ((UINT16)0x00c0)
	tg04_00_00, tg04_00_01, tg04_00_02, tg04_00_03, tg04_00_04, tg04_00_05, tg04_00_06, tg04_00_07, tg04_00_08, tg04_00_09, tg04_00_0a, tg04_00_0b, tg04_00_0c, tg04_00_0d, tg04_00_0e, tg04_00_0f, tg04_01_00, tg04_01_01, tg04_01_02, tg04_01_03, tg04_01_10, tg04_01_11, tg04_01_12, tg04_01_13, tg04_01_20, tg04_01_21, tg04_01_22, tg04_01_23, tg04_01_30, tg04_01_31, tg04_01_32, tg04_01_33, tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,  tg04_0203,
//#define THUMB_GROUP5_TYPE   ((UINT16)0x0e00)
	tg05_0,     tg05_0,     tg05_0,     tg05_0,     tg05_0,     tg05_0,     tg05_0,     tg05_0,     tg05_1,     tg05_1,     tg05_1,     tg05_1,     tg05_1,     tg05_1,     tg05_1,     tg05_1,     tg05_2,     tg05_2,     tg05_2,     tg05_2,     tg05_2,     tg05_2,     tg05_2,     tg05_2,     tg05_3,     tg05_3,     tg05_3,     tg05_3,     tg05_3,     tg05_3,     tg05_3,     tg05_3,     tg05_4,     tg05_4,     tg05_4,     tg05_4,     tg05_4,     tg05_4,     tg05_4,     tg05_4,     tg05_5,     tg05_5,     tg05_5,     tg05_5,     tg05_5,     tg05_5,     tg05_5,     tg05_5,     tg05_6,     tg05_6,     tg05_6,     tg05_6,     tg05_6,     tg05_6,     tg05_6,     tg05_6,     tg05_7,     tg05_7,     tg05_7,     tg05_7,     tg05_7,     tg05_7,     tg05_7,     tg05_7,
//#define THUMB_LSOP_L        ((UINT16)0x0800)
	tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_0,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,     tg06_1,
//#define THUMB_LSOP_L        ((UINT16)0x0800)
	tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_0,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,     tg07_1,
// #define THUMB_HALFOP_L      ((UINT16)0x0800)
	tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_0,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,     tg08_1,
// #define THUMB_STACKOP_L     ((UINT16)0x0800)
	tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_0,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,     tg09_1,
// #define THUMB_RELADDR_SP    ((UINT16)0x0800)
	tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_0,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,     tg0a_1,
// #define THUMB_STACKOP_TYPE  ((UINT16)0x0f00)
	tg0b_0,     tg0b_0,     tg0b_0,     tg0b_0,     tg0b_1,     tg0b_1,     tg0b_1,     tg0b_1,     tg0b_2,     tg0b_2,     tg0b_2,     tg0b_2,     tg0b_3,     tg0b_3,     tg0b_3,     tg0b_3,     tg0b_4,     tg0b_4,     tg0b_4,     tg0b_4,     tg0b_5,     tg0b_5,     tg0b_5,     tg0b_5,     tg0b_6,     tg0b_6,     tg0b_6,     tg0b_6,     tg0b_7,     tg0b_7,     tg0b_7,     tg0b_7,     tg0b_8,     tg0b_8,     tg0b_8,     tg0b_8,     tg0b_9,     tg0b_9,     tg0b_9,     tg0b_9,     tg0b_a,     tg0b_a,     tg0b_a,     tg0b_a,     tg0b_b,     tg0b_b,     tg0b_b,     tg0b_b,     tg0b_c,     tg0b_c,     tg0b_c,     tg0b_c,     tg0b_d,     tg0b_d,     tg0b_d,     tg0b_d,     tg0b_e,     tg0b_e,     tg0b_e,     tg0b_e,     tg0b_f,     tg0b_f,     tg0b_f,     tg0b_f,
// #define THUMB_MULTLS        ((UINT16)0x0800)
	tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_0,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,     tg0c_1,
// #define THUMB_COND_TYPE     ((UINT16)0x0f00)
	tg0d_0,     tg0d_0,     tg0d_0,     tg0d_0,     tg0d_1,     tg0d_1,     tg0d_1,     tg0d_1,     tg0d_2,     tg0d_2,     tg0d_2,     tg0d_2,     tg0d_3,     tg0d_3,     tg0d_3,     tg0d_3,     tg0d_4,     tg0d_4,     tg0d_4,     tg0d_4,     tg0d_5,     tg0d_5,     tg0d_5,     tg0d_5,     tg0d_6,     tg0d_6,     tg0d_6,     tg0d_6,     tg0d_7,     tg0d_7,     tg0d_7,     tg0d_7,     tg0d_8,     tg0d_8,     tg0d_8,     tg0d_8,     tg0d_9,     tg0d_9,     tg0d_9,     tg0d_9,     tg0d_a,     tg0d_a,     tg0d_a,     tg0d_a,     tg0d_b,     tg0d_b,     tg0d_b,     tg0d_b,     tg0d_c,     tg0d_c,     tg0d_c,     tg0d_c,     tg0d_d,     tg0d_d,     tg0d_d,     tg0d_d,     tg0d_e,     tg0d_e,     tg0d_e,     tg0d_e,     tg0d_f,     tg0d_f,     tg0d_f,     tg0d_f,
// #define THUMB_BLOP_LO       ((UINT16)0x0800)
	tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_0,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,     tg0e_1,
// #define THUMB_BLOP_LO       ((UINT16)0x0800)
	tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_0,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,     tg0f_1,
};

	/* Shift operations */

const void tg00_0(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Shift left */
{
	UINT32 rs, rd, rrs;
	INT32 offs;

	SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rrs = GET_REGISTER(cpustate, rs);
	offs = (insn & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
	if (offs != 0)
	{
		SET_REGISTER(cpustate, rd, rrs << offs);
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
		SET_REGISTER(cpustate, rd, rrs);
	}
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
	R15 += 2;
}

const void tg00_1(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Shift right */
{
	UINT32 rs, rd, rrs;
	INT32 offs;

	SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rrs = GET_REGISTER(cpustate, rs);
	offs = (insn & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
	if (offs != 0)
	{
		SET_REGISTER(cpustate, rd, rrs >> offs);
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
		SET_REGISTER(cpustate, rd, 0);
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
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
	R15 += 2;
}

/* Arithmetic */

const void tg01_0(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
	UINT32 rs, rd, rrs;
	INT32 offs;
	/* ASR.. */
	//if (insn & THUMB_SHIFT_R) /* Shift right */
	{
		rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
		rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
		rrs = GET_REGISTER(cpustate, rs);
		offs = (insn & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
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
			SET_REGISTER(cpustate, rd, (rrs & 0x80000000) ? 0xFFFFFFFF : 0x00000000);
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
			SET_REGISTER(cpustate, rd,
							(rrs & 0x80000000)
							? ((0xFFFFFFFF << (32 - offs)) | (rrs >> offs))
							: (rrs >> offs));
		}
		SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));
		SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
		R15 += 2;
	}
}

const void tg01_10(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* ADD Rd, Rs, Rn */
{
	UINT32 rn, rs, rd;


	rn = GET_REGISTER(cpustate, (insn & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT);
	rs = GET_REGISTER(cpustate, (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(cpustate, rd, rs + rn);
	HandleThumbALUAddFlags(GET_REGISTER(cpustate, rd), rs, rn);

}

const void tg01_11(arm_state *cpustate, UINT32 pc, UINT32 insn) /* SUB Rd, Rs, Rn */
{
	UINT32 rn, rs, rd;

	rn = GET_REGISTER(cpustate, (insn & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT);
	rs = GET_REGISTER(cpustate, (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(cpustate, rd, rs - rn);
	HandleThumbALUSubFlags(GET_REGISTER(cpustate, rd), rs, rn);

}

const void tg01_12(arm_state *cpustate, UINT32 pc, UINT32 insn) /* ADD Rd, Rs, #imm */
{
	UINT32 rs, rd, imm;

	imm = (insn & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT;
	rs = GET_REGISTER(cpustate, (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(cpustate, rd, rs + imm);
	HandleThumbALUAddFlags(GET_REGISTER(cpustate, rd), rs, imm);

}

const void tg01_13(arm_state *cpustate, UINT32 pc, UINT32 insn) /* SUB Rd, Rs, #imm */
{
	UINT32 rs, rd, imm;

	imm = (insn & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT;
	rs = GET_REGISTER(cpustate, (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(cpustate, rd, rs - imm);
	HandleThumbALUSubFlags(GET_REGISTER(cpustate, rd), rs,imm);

}

/* CMP / MOV */

const void tg02_0(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
	UINT32 rd, op2;

	rd = (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT;
	op2 = (insn & THUMB_INSN_IMM);
	SET_REGISTER(cpustate, rd, op2);
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
	R15 += 2;
}

const void tg02_1(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
	UINT32 rn, rd, op2;

	rn = GET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
	op2 = insn & THUMB_INSN_IMM;
	rd = rn - op2;
	HandleThumbALUSubFlags(rd, rn, op2);
	//mame_printf_debug("%08x: xxx Thumb instruction: CMP R%d (%08x), %02x (N=%d, Z=%d, C=%d, V=%d)\n", pc, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, GET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT), op2, N_IS_SET(GET_CPSR) ? 1 : 0, Z_IS_SET(GET_CPSR) ? 1 : 0, C_IS_SET(GET_CPSR) ? 1 : 0, V_IS_SET(GET_CPSR) ? 1 : 0);
}



/* ADD/SUB immediate */

const void tg03_0(arm_state *cpustate, UINT32 pc, UINT32 insn) /* ADD Rd, #Offset8 */
{
	UINT32 rn, rd, op2;

	rn = GET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
	op2 = insn & THUMB_INSN_IMM;
	rd = rn + op2;
	//mame_printf_debug("%08x:  Thumb instruction: ADD R%d, %02x\n", pc, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, op2);
	SET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, rd);
	HandleThumbALUAddFlags(rd, rn, op2);
}

const void tg03_1(arm_state *cpustate, UINT32 pc, UINT32 insn) /* SUB Rd, #Offset8 */
{
	UINT32 rn, rd, op2;

	rn = GET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
	op2 = insn & THUMB_INSN_IMM;
	//mame_printf_debug("%08x:  Thumb instruction: SUB R%d, %02x\n", pc, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, op2);
	rd = rn - op2;
	SET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, rd);
	HandleThumbALUSubFlags(rd, rn, op2);
}



/* Rd & Rm instructions */

const void tg04_00_00(arm_state *cpustate, UINT32 pc, UINT32 insn) /* AND Rd, Rs */
{
	UINT32 rs, rd;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rd) & GET_REGISTER(cpustate, rs));
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
	R15 += 2;

}

const void tg04_00_01(arm_state *cpustate, UINT32 pc, UINT32 insn) /* EOR Rd, Rs */
{
	UINT32 rs, rd;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rd) ^ GET_REGISTER(cpustate, rs));
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
	R15 += 2;

}

const void tg04_00_02(arm_state *cpustate, UINT32 pc, UINT32 insn) /* LSL Rd, Rs */
{
	UINT32 rs, rd, rrd;
	INT32 offs;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rrd = GET_REGISTER(cpustate, rd);
	offs = GET_REGISTER(cpustate, rs) & 0x000000ff;
	if (offs > 0)
	{
		if (offs < 32)
		{
			SET_REGISTER(cpustate, rd, rrd << offs);
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
			SET_REGISTER(cpustate, rd, 0);
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
			SET_REGISTER(cpustate, rd, 0);
			SET_CPSR(GET_CPSR & ~C_MASK);
		}
	}
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
	R15 += 2;

}

const void tg04_00_03(arm_state *cpustate, UINT32 pc, UINT32 insn) /* LSR Rd, Rs */
{
	UINT32 rs, rd, rrd;
	INT32 offs;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rrd = GET_REGISTER(cpustate, rd);
	offs = GET_REGISTER(cpustate, rs) & 0x000000ff;
	if (offs >  0)
	{
		if (offs < 32)
		{
			SET_REGISTER(cpustate, rd, rrd >> offs);
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
			SET_REGISTER(cpustate, rd, 0);
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
			SET_REGISTER(cpustate, rd, 0);
			SET_CPSR(GET_CPSR & ~C_MASK);
		}
	}
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
	R15 += 2;

}

const void tg04_00_04(arm_state *cpustate, UINT32 pc, UINT32 insn) /* ASR Rd, Rs */
{
	UINT32 rs, rd, rrs, rrd;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rrs = GET_REGISTER(cpustate, rs)&0xff;
	rrd = GET_REGISTER(cpustate, rd);
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
			SET_REGISTER(cpustate, rd, (GET_REGISTER(cpustate, rd) & 0x80000000) ? 0xFFFFFFFF : 0x00000000);
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
			SET_REGISTER(cpustate, rd, (rrd & 0x80000000)
							? ((0xFFFFFFFF << (32 - rrs)) | (rrd >> rrs))
							: (rrd >> rrs));
		}
	}
	SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
	R15 += 2;

}

const void tg04_00_05(arm_state *cpustate, UINT32 pc, UINT32 insn) /* ADC Rd, Rs */
{
	UINT32 rn, rs, rd, op2;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	op2=(GET_CPSR & C_MASK) ? 1 : 0;
	rn=GET_REGISTER(cpustate, rd) + GET_REGISTER(cpustate, rs) + op2;
	HandleThumbALUAddFlags(rn, GET_REGISTER(cpustate, rd), (GET_REGISTER(cpustate, rs))); // ?
	SET_REGISTER(cpustate, rd, rn);

}

const void tg04_00_06(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* SBC Rd, Rs */
{
	UINT32 rn, rs, rd, op2;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	op2=(GET_CPSR & C_MASK) ? 0 : 1;
	rn=GET_REGISTER(cpustate, rd) - GET_REGISTER(cpustate, rs) - op2;
	HandleThumbALUSubFlags(rn, GET_REGISTER(cpustate, rd), (GET_REGISTER(cpustate, rs))); //?
	SET_REGISTER(cpustate, rd, rn);

}

const void tg04_00_07(arm_state *cpustate, UINT32 pc, UINT32 insn) /* ROR Rd, Rs */
{
	UINT32 rs, rd, imm, rrd;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rrd = GET_REGISTER(cpustate, rd);
	imm = GET_REGISTER(cpustate, rs) & 0x0000001f;
	SET_REGISTER(cpustate, rd, (rrd >> imm) | (rrd << (32 - imm)));
	if (rrd & (1 << (imm - 1)))
	{
		SET_CPSR(GET_CPSR | C_MASK);
	}
	else
	{
		SET_CPSR(GET_CPSR & ~C_MASK);
	}
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
	R15 += 2;

}

const void tg04_00_08(arm_state *cpustate, UINT32 pc, UINT32 insn) /* TST Rd, Rs */
{
	UINT32 rs, rd;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd) & GET_REGISTER(cpustate, rs)));
	R15 += 2;

}

const void tg04_00_09(arm_state *cpustate, UINT32 pc, UINT32 insn) /* NEG Rd, Rs */
{
	UINT32 rn, rs, rd, rrs;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rrs = GET_REGISTER(cpustate, rs);
	rn = 0 - rrs;
	SET_REGISTER(cpustate, rd, rn);
	HandleThumbALUSubFlags(GET_REGISTER(cpustate, rd), 0, rrs);

}

const void tg04_00_0a(arm_state *cpustate, UINT32 pc, UINT32 insn) /* CMP Rd, Rs */
{
	UINT32 rn, rs, rd;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rn = GET_REGISTER(cpustate, rd) - GET_REGISTER(cpustate, rs);
	HandleThumbALUSubFlags(rn, GET_REGISTER(cpustate, rd), GET_REGISTER(cpustate, rs));

}


const void tg04_00_0b(arm_state *cpustate, UINT32 pc, UINT32 insn) /* CMN Rd, Rs - check flags, add dasm */
{
	UINT32 rn, rs, rd;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rn = GET_REGISTER(cpustate, rd) + GET_REGISTER(cpustate, rs);
	HandleThumbALUAddFlags(rn, GET_REGISTER(cpustate, rd), GET_REGISTER(cpustate, rs));

}

const void tg04_00_0c(arm_state *cpustate, UINT32 pc, UINT32 insn) /* ORR Rd, Rs */
{
	UINT32 rs, rd;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rd) | GET_REGISTER(cpustate, rs));
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
	R15 += 2;

}

const void tg04_00_0d(arm_state *cpustate, UINT32 pc, UINT32 insn) /* MUL Rd, Rs */
{
	UINT32 rn, rs, rd;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	rn = GET_REGISTER(cpustate, rd) * GET_REGISTER(cpustate, rs);
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_REGISTER(cpustate, rd, rn);
	SET_CPSR(GET_CPSR | HandleALUNZFlags(rn));
	R15 += 2;

}

const void tg04_00_0e(arm_state *cpustate, UINT32 pc, UINT32 insn) /* BIC Rd, Rs */
{
	UINT32 rs, rd;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rd) & (~GET_REGISTER(cpustate, rs)));
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
	R15 += 2;

}
const void tg04_00_0f(arm_state *cpustate, UINT32 pc, UINT32 insn) /* MVN Rd, Rs */
{
	UINT32 rs, rd, op2;

	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	op2 = GET_REGISTER(cpustate, rs);
	SET_REGISTER(cpustate, rd, ~op2);
	SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
	SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
	R15 += 2;

}

/* ADD Rd, Rs group */

const void tg04_01_00(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 rs, rd;
//  rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
//  rd = insn & THUMB_HIREG_RD;


	fatalerror("%08x: G4-1-0 Undefined Thumb instruction: %04x %x\n", pc, insn, (insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT);

	R15 += 2;

}

const void tg04_01_01(arm_state *cpustate, UINT32 pc, UINT32 insn) /* ADD Rd, HRs */
{
	UINT32 rs, rd;
	rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	rd = insn & THUMB_HIREG_RD;


	SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rd) + GET_REGISTER(cpustate, rs+8));
	// emulate the effects of pre-fetch
	if (rs == 7)
	{
		SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rd) + 4);
	}

	R15 += 2;
}

const void tg04_01_02(arm_state *cpustate, UINT32 pc, UINT32 insn) /* ADD HRd, Rs */
{
	UINT32 rs, rd;
	rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	rd = insn & THUMB_HIREG_RD;


	SET_REGISTER(cpustate, rd+8, GET_REGISTER(cpustate, rd+8) + GET_REGISTER(cpustate, rs));
	if (rd == 7)
	{
		R15 += 2;
	}

	R15 += 2;
}

const void tg04_01_03(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Add HRd, HRs */
{
	UINT32 rs, rd;
	rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	rd = insn & THUMB_HIREG_RD;


	SET_REGISTER(cpustate, rd+8, GET_REGISTER(cpustate, rd+8) + GET_REGISTER(cpustate, rs+8));
	// emulate the effects of pre-fetch
	if (rs == 7)
	{
		SET_REGISTER(cpustate, rd+8, GET_REGISTER(cpustate, rd+8) + 4);
	}
	if (rd == 7)
	{
		R15 += 2;
	}

	R15 += 2;
}


const void tg04_01_10(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* CMP Rd, Rs */
{
	UINT32 rn, rs, rd;

	rs = GET_REGISTER(cpustate, ((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT));
	rd = GET_REGISTER(cpustate, insn & THUMB_HIREG_RD);
	rn = rd - rs;
	HandleThumbALUSubFlags(rn, rd, rs);

}

const void tg04_01_11(arm_state *cpustate, UINT32 pc, UINT32 insn) /* CMP Rd, Hs */
{
	UINT32 rn, rs, rd;

	rs = GET_REGISTER(cpustate, ((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
	rd = GET_REGISTER(cpustate, insn & THUMB_HIREG_RD);
	rn = rd - rs;
	HandleThumbALUSubFlags(rn, rd, rs);

}

const void tg04_01_12(arm_state *cpustate, UINT32 pc, UINT32 insn) /* CMP Hd, Rs */
{
	UINT32 rn, rs, rd;

	rs = GET_REGISTER(cpustate, ((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT));
	rd = GET_REGISTER(cpustate, (insn & THUMB_HIREG_RD) + 8);
	rn = rd - rs;
	HandleThumbALUSubFlags(rn, rd, rs);

}

const void tg04_01_13(arm_state *cpustate, UINT32 pc, UINT32 insn) /* CMP Hd, Hs */
{
	UINT32 rn, rs, rd;

	rs = GET_REGISTER(cpustate, ((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
	rd = GET_REGISTER(cpustate, (insn & THUMB_HIREG_RD) + 8);
	rn = rd - rs;
	HandleThumbALUSubFlags(rn, rd, rs);

}

	/* MOV group */

// "The action of H1 = 0, H2 = 0 for Op = 00 (ADD), Op = 01 (CMP) and Op = 10 (MOV) is undefined, and should not be used."
const void tg04_01_20(arm_state *cpustate, UINT32 pc, UINT32 insn) /* MOV Rd, Rs (undefined) */
{
	UINT32 rs, rd;


	rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	rd = insn & THUMB_HIREG_RD;
	SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rs));
	R15 += 2;

}

const void tg04_01_21(arm_state *cpustate, UINT32 pc, UINT32 insn) /* MOV Rd, Hs */
{
	UINT32 rs, rd;

	rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	rd = insn & THUMB_HIREG_RD;
	if (rs == 7)
	{
		SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rs + 8) + 4);
	}
	else
	{
		SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rs + 8));
	}
	R15 += 2;

}

const void tg04_01_22(arm_state *cpustate, UINT32 pc, UINT32 insn) /* MOV Hd, Rs */
{
	UINT32 rs, rd;

	rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	rd = insn & THUMB_HIREG_RD;
	SET_REGISTER(cpustate, rd + 8, GET_REGISTER(cpustate, rs));
	if (rd != 7)
	{
		R15 += 2;
	}
	else
	{
		R15 &= ~1;
	}

}

const void tg04_01_23(arm_state *cpustate, UINT32 pc, UINT32 insn) /* MOV Hd, Hs */
{
	UINT32 rs, rd;

	rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	rd = insn & THUMB_HIREG_RD;
	if (rs == 7)
	{
		SET_REGISTER(cpustate, rd + 8, GET_REGISTER(cpustate, rs+8)+4);
	}
	else
	{
		SET_REGISTER(cpustate, rd + 8, GET_REGISTER(cpustate, rs+8));
	}
	if (rd != 7)
	{
		R15 += 2;
	}
	if (rd == 7)
	{
		R15 &= ~1;
	}

}


const void tg04_01_30(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
	UINT32 addr;
	UINT32 rd;


	rd = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
	addr = GET_REGISTER(cpustate, rd);
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

const void tg04_01_31(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
	UINT32 addr;


	addr = GET_REGISTER(cpustate, ((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
	if ((((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8) == 15)
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

const void tg04_01_32(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  UINT32 rd;


	fatalerror("%08x: G4-3 Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}

const void tg04_01_33(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  UINT32 rd;


	fatalerror("%08x: G4-3 Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}






const void tg04_0203(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
	UINT32 readword;

	readword = READ32((R15 & ~2) + 4 + ((insn & THUMB_INSN_IMM) << 2));
	SET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, readword);
	R15 += 2;
}

/* LDR* STR* group */

const void tg05_0(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* STR Rd, [Rn, Rm] */
{
	UINT32 addr;
	UINT32 rm, rn, rd;

	rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
	WRITE32(addr, GET_REGISTER(cpustate, rd));
	R15 += 2;

}

const void tg05_1(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* STRH Rd, [Rn, Rm] */
{
	UINT32 addr;
	UINT32 rm, rn, rd;

	rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
	WRITE16(addr, GET_REGISTER(cpustate, rd));
	R15 += 2;

}

const void tg05_2(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* STRB Rd, [Rn, Rm] */
{
	UINT32 addr;
	UINT32 rm, rn, rd;

	rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
	WRITE8(addr, GET_REGISTER(cpustate, rd));
	R15 += 2;

}

const void tg05_3(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* LDSB Rd, [Rn, Rm] todo, add dasm */
{
	UINT32 addr;
	UINT32 rm, rn, rd, op2;

	rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
	op2 = READ8(addr);
	if (op2 & 0x00000080)
	{
		op2 |= 0xffffff00;
	}
	SET_REGISTER(cpustate, rd, op2);
	R15 += 2;

}

const void tg05_4(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* LDR Rd, [Rn, Rm] */
{
	UINT32 addr;
	UINT32 rm, rn, rd, op2;

	rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
	op2 = READ32(addr);
	SET_REGISTER(cpustate, rd, op2);
	R15 += 2;

}

const void tg05_5(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* LDRH Rd, [Rn, Rm] */
{
	UINT32 addr;
	UINT32 rm, rn, rd, op2;

	rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
	op2 = READ16(addr);
	SET_REGISTER(cpustate, rd, op2);
	R15 += 2;

}

const void tg05_6(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* LDRB Rd, [Rn, Rm] */
{
	UINT32 addr;
	UINT32 rm, rn, rd, op2;

	rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
	op2 = READ8(addr);
	SET_REGISTER(cpustate, rd, op2);
	R15 += 2;

}

const void tg05_7(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* LDSH Rd, [Rn, Rm] */
{
	UINT32 addr;
	UINT32 rm, rn, rd, op2;

	rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
	rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
	rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
	addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
	op2 = READ16(addr);
	if (op2 & 0x00008000)
	{
		op2 |= 0xffff0000;
	}
	SET_REGISTER(cpustate, rd, op2);
	R15 += 2;

}

	/* Word Store w/ Immediate Offset */

const void tg06_0(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Store */
{
	UINT32 rn, rd;
	INT32 offs;

	rn = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = insn & THUMB_ADDSUB_RD;
	offs = ((insn & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT) << 2;
	WRITE32(GET_REGISTER(cpustate, rn) + offs, GET_REGISTER(cpustate, rd));
	R15 += 2;
}

const void tg06_1(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Load */
{
	UINT32 rn, rd;
	INT32 offs;

	rn = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = insn & THUMB_ADDSUB_RD;
	offs = ((insn & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT) << 2;
	SET_REGISTER(cpustate, rd, READ32(GET_REGISTER(cpustate, rn) + offs)); // fix
	R15 += 2;
}

/* Byte Store w/ Immeidate Offset */

const void tg07_0(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Store */
{
	UINT32 rn, rd;
	INT32 offs;

	rn = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = insn & THUMB_ADDSUB_RD;
	offs = (insn & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT;
	WRITE8(GET_REGISTER(cpustate, rn) + offs, GET_REGISTER(cpustate, rd));
	R15 += 2;
}

const void tg07_1(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* Load */
{
	UINT32 rn, rd;
	INT32 offs;

	rn = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = insn & THUMB_ADDSUB_RD;
	offs = (insn & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT;
	SET_REGISTER(cpustate, rd, READ8(GET_REGISTER(cpustate, rn) + offs));
	R15 += 2;
}

/* Load/Store Halfword */

const void tg08_0(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Store */
{
	UINT32 rs, rd, imm;

	imm = (insn & THUMB_HALFOP_OFFS) >> THUMB_HALFOP_OFFS_SHIFT;
	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	WRITE16(GET_REGISTER(cpustate, rs) + (imm << 1), GET_REGISTER(cpustate, rd));
	R15 += 2;
}

const void tg08_1(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Load */
{
	UINT32 rs, rd, imm;

	imm = (insn & THUMB_HALFOP_OFFS) >> THUMB_HALFOP_OFFS_SHIFT;
	rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
	rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
	SET_REGISTER(cpustate, rd, READ16(GET_REGISTER(cpustate, rs) + (imm << 1)));
	R15 += 2;
}

	/* Stack-Relative Load/Store */

const void tg09_0(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Store */
{
	UINT32 rd;
	INT32 offs;

	rd = (insn & THUMB_STACKOP_RD) >> THUMB_STACKOP_RD_SHIFT;
	offs = (UINT8)(insn & THUMB_INSN_IMM);
	WRITE32(GET_REGISTER(cpustate, 13) + ((UINT32)offs << 2), GET_REGISTER(cpustate, rd));
	R15 += 2;
}

const void tg09_1(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Load */
{
	UINT32 readword;
	UINT32 rd;
	INT32 offs;

	rd = (insn & THUMB_STACKOP_RD) >> THUMB_STACKOP_RD_SHIFT;
	offs = (UINT8)(insn & THUMB_INSN_IMM);
	readword = READ32(GET_REGISTER(cpustate, 13) + ((UINT32)offs << 2));
	SET_REGISTER(cpustate, rd, readword);
	R15 += 2;
}

	/* Get relative address */

const void tg0a_0(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* ADD Rd, PC, #nn */
{
	UINT32 rd;
	INT32 offs;

	rd = (insn & THUMB_RELADDR_RD) >> THUMB_RELADDR_RD_SHIFT;
	offs = (UINT8)(insn & THUMB_INSN_IMM) << 2;
	SET_REGISTER(cpustate, rd, ((R15 + 4) & ~2) + offs);
	R15 += 2;
}

const void tg0a_1(arm_state *cpustate, UINT32 pc, UINT32 insn) /* ADD Rd, SP, #nn */
{
	UINT32 rd;
	INT32 offs;

	rd = (insn & THUMB_RELADDR_RD) >> THUMB_RELADDR_RD_SHIFT;
	offs = (UINT8)(insn & THUMB_INSN_IMM) << 2;
	SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, 13) + offs);
	R15 += 2;
}

	/* Stack-Related Opcodes */

const void tg0b_0(arm_state *cpustate, UINT32 pc, UINT32 insn) /* ADD SP, #imm */
{
	UINT32 addr;


	addr = (insn & THUMB_INSN_IMM);
	addr &= ~THUMB_INSN_IMM_S;
	SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) + ((insn & THUMB_INSN_IMM_S) ? -(addr << 2) : (addr << 2)));
	R15 += 2;

}

const void tg0b_1(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  INT32 offs;

	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}

const void tg0b_2(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  INT32 offs;

	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}

const void tg0b_3(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  INT32 offs;

	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}

const void tg0b_4(arm_state *cpustate, UINT32 pc, UINT32 insn) /* PUSH {Rlist} */
{
	INT32 offs;

	for (offs = 7; offs >= 0; offs--)
	{
		if (insn & (1 << offs))
		{
			SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) - 4);
			WRITE32(GET_REGISTER(cpustate, 13), GET_REGISTER(cpustate, offs));
		}
	}
	R15 += 2;

}

const void tg0b_5(arm_state *cpustate, UINT32 pc, UINT32 insn) /* PUSH {Rlist}{LR} */
{
	INT32 offs;

	SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) - 4);
	WRITE32(GET_REGISTER(cpustate, 13), GET_REGISTER(cpustate, 14));
	for (offs = 7; offs >= 0; offs--)
	{
		if (insn & (1 << offs))
		{
			SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) - 4);
			WRITE32(GET_REGISTER(cpustate, 13), GET_REGISTER(cpustate, offs));
		}
	}
	R15 += 2;

}

const void tg0b_6(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  INT32 offs;

	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}

const void tg0b_7(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  INT32 offs;

	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}

const void tg0b_8(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  INT32 offs;

	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}

const void tg0b_9(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  INT32 offs;

	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}

const void tg0b_a(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  INT32 offs;

	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}

const void tg0b_b(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  INT32 offs;

	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}

const void tg0b_c(arm_state *cpustate, UINT32 pc, UINT32 insn) /* POP {Rlist} */
{
	INT32 offs;

	for (offs = 0; offs < 8; offs++)
	{
		if (insn & (1 << offs))
		{
			SET_REGISTER(cpustate, offs, READ32(GET_REGISTER(cpustate, 13)));
			SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) + 4);
		}
	}
	R15 += 2;

}

const void tg0b_d(arm_state *cpustate, UINT32 pc, UINT32 insn) /* POP {Rlist}{PC} */
{
	INT32 offs;

	for (offs = 0; offs < 8; offs++)
	{
		if (insn & (1 << offs))
		{
			SET_REGISTER(cpustate, offs, READ32(GET_REGISTER(cpustate, 13)));
			SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) + 4);
		}
	}
	UINT32 addr = READ32(GET_REGISTER(cpustate, 13));
	// in v4T, bit 0 is ignored.  v5 and later, it's an ARM/Thumb flag like the BX instruction
	if (cpustate->archRev < 5)
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
	SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) + 4);

}

const void tg0b_e(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  INT32 offs;

	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}

const void tg0b_f(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
//  UINT32 addr;
//  INT32 offs;

	fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
	R15 += 2;

}

/* Multiple Load/Store */

// "The address should normally be a word aligned quantity and non-word aligned addresses do not affect the instruction."
// "However, the bottom 2 bits of the address will appear on A[1:0] and might be interpreted by the memory system."

// GBA "BB Ball" performs an unaligned read with A[1:0] = 2 and expects A[1] not to be ignored [BP 800B90A,(R4&3)!=0]
// GBA "Gadget Racers" performs an unaligned read with A[1:0] = 1 and expects A[0] to be ignored [BP B72,(R0&3)!=0]

const void tg0c_0(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Store */
{
	UINT32 rd;
	INT32 offs;

	UINT32 ld_st_address;

	rd = (insn & THUMB_MULTLS_BASE) >> THUMB_MULTLS_BASE_SHIFT;


	ld_st_address = GET_REGISTER(cpustate, rd);

	for (offs = 0; offs < 8; offs++)
	{
		if (insn & (1 << offs))
		{
			WRITE32(ld_st_address & ~3, GET_REGISTER(cpustate, offs));
			ld_st_address += 4;
		}
	}
	SET_REGISTER(cpustate, rd, ld_st_address);
	R15 += 2;
}


const void tg0c_1(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Load */
{
	UINT32 rd;
	INT32 offs;

	UINT32 ld_st_address;

	rd = (insn & THUMB_MULTLS_BASE) >> THUMB_MULTLS_BASE_SHIFT;


	ld_st_address = GET_REGISTER(cpustate, rd);


	int rd_in_list;

	rd_in_list = insn & (1 << rd);
	for (offs = 0; offs < 8; offs++)
	{
		if (insn & (1 << offs))
		{
			SET_REGISTER(cpustate, offs, READ32(ld_st_address & ~1));
			ld_st_address += 4;
		}
	}

	if (!rd_in_list)
		SET_REGISTER(cpustate, rd, ld_st_address);

	R15 += 2;
}

/* Conditional Branch */

const void tg0d_0(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_EQ:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (Z_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_1(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_NE:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (Z_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_2(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_CS:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (C_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_3(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_CC:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (C_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_4(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_MI:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (N_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_5(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_PL:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (N_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_6(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_VS:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (V_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_7(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_VC:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (V_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_8(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_HI:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (C_IS_SET(GET_CPSR) && Z_IS_CLEAR(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_9(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_LS:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (C_IS_CLEAR(GET_CPSR) || Z_IS_SET(GET_CPSR))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_a(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_GE:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (!(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_b(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_LT:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (!(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_c(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_GT:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (Z_IS_CLEAR(GET_CPSR) && !(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_d(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_LE:
{
	INT32 offs;

	offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	if (Z_IS_SET(GET_CPSR) || !(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK))
	{
		R15 += 4 + (offs << 1);
	}
	else
	{
		R15 += 2;
	}

}

const void tg0d_e(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_AL:
{
//  INT32 offs;

//    offs = (INT8)(insn & THUMB_INSN_IMM);
//case
	fatalerror("%08x: Undefined Thumb instruction: %04x (ARM9 reserved)\n", pc, insn);
	R15 += 2;

}

const void tg0d_f(arm_state *cpustate, UINT32 pc, UINT32 insn) // COND_NV:   // SWI (this is sort of a "hole" in the opcode encoding)
{
//  INT32 offs;

//    offs = (INT8)(insn & THUMB_INSN_IMM);

//case
	cpustate->pendingSwi = 1;
	ARM7_CHECKIRQ;

}

	/* B #offs */

const void tg0e_0(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
	INT32 offs;

	offs = (insn & THUMB_BRANCH_OFFS) << 1;
	if (offs & 0x00000800)
	{
		offs |= 0xfffff800;
	}
	R15 += 4 + offs;
}


const void tg0e_1(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
	UINT32 addr;

	addr = GET_REGISTER(cpustate, 14);
	addr += (insn & THUMB_BLOP_OFFS) << 1;
	addr &= 0xfffffffc;
	SET_REGISTER(cpustate, 14, (R15 + 4) | 1);
	R15 = addr;
}

	/* BL */

const void tg0f_0(arm_state *cpustate, UINT32 pc, UINT32 insn)
{
	UINT32 addr;

	addr = (insn & THUMB_BLOP_OFFS) << 12;
	if (addr & (1 << 22))
	{
		addr |= 0xff800000;
	}
	addr += R15 + 4;
	SET_REGISTER(cpustate, 14, addr);
	R15 += 2;
}


const void tg0f_1(arm_state *cpustate, UINT32 pc, UINT32 insn) /* BL */
{
	UINT32 addr;

	addr = GET_REGISTER(cpustate, 14) & ~1;
	addr += (insn & THUMB_BLOP_OFFS) << 1;
	SET_REGISTER(cpustate, 14, (R15 + 2) | 1);
	R15 = addr;
	//R15 += 2;
}
