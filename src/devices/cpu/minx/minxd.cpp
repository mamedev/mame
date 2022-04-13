// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/************************************************************

  Nintendo Minx CPU disassembly


************************************************************/

#include "emu.h"
#include "minxd.h"


const char *const minx_disassembler::s_mnemonic[] = {
	"add", "addc", "and", "bcdd", "bcde", "bcdx", "call", "callc", "callg", "callge", "calll",
	"callle", "calln", "callnc", "callno", "callnz", "callo", "callp", "callnx0",
	"callnx1", "callnx2", "callnx3", "callx0", "callx1", "callx2", "callx3", "callz",
	"cmp", "cmpn", "dec", "div", "ext", "halt", "inc", "int",
	"jc", "jdbnz", "jg", "jge", "jint", "jl", "jle", "jmp",
	"jn", "jnx0", "jnx1", "jnx2", "jnx3", "jnc", "jno", "jnz",
	"jo", "jp", "jx0", "jx1", "jx2", "jx3", "jz", "mov",
	"mul", "neg", "nop", "not", "or", "pop", "popa", "popax",
	"popx", "push", "pusha", "pushax", "pushx", "ret", "reti", "retskip",
	"rol", "rolc", "ror", "rorc", "sal", "sar", "shl", "shr", "sub",
	"subc", "test", "xchg", "xor", "db"
};

const uint32_t minx_disassembler::s_flags[] = {
	0, 0, 0, 0, 0, 0, STEP_OVER, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND,
	STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND,
	STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND,
	0, 0, 0, 0, 0, STEP_OVER, 0, STEP_OVER,
	STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, 0,
	STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND,
	STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, STEP_OUT, STEP_OUT, STEP_OUT,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
};

const minx_disassembler::minxdasm minx_disassembler::mnemonic[256] = {
	/* 00 - 0F */
	{zADD,R_A,R_A},    {zADD,R_A,R_B},     {zADD,R_A,I_8},    {zADD,R_A,M_IHL},
	{zADD,R_A,M_N8},   {zADD,R_A,M_I16},   {zADD,R_A,M_X},    {zADD,R_A,M_Y},
	{zADDC,R_A,R_A},   {zADDC,R_A,R_B},    {zADDC,R_A,I_8},   {zADDC,R_A,M_IHL},
	{zADDC,R_A,M_N8},  {zADDC,R_A,M_I16},  {zADDC,R_A,M_X},   {zADDC,R_A,M_Y},
	/* 10 - 1F */
	{zSUB,R_A,R_A},    {zSUB,R_A,R_B},     {zSUB,R_A,I_8},    {zSUB,R_A,M_IHL},
	{zSUB,R_A,M_N8},   {zSUB,R_A,M_I16},   {zSUB,R_A,M_X},    {zSUB,R_A,M_Y},
	{zSUBC,R_A,R_A},   {zSUBC,R_A,R_B},    {zSUBC,R_A,I_8},   {zSUBC,R_A,M_IHL},
	{zSUBC,R_A,M_N8},  {zSUBC,R_A,M_I16},  {zSUBC,R_A,M_X},   {zSUBC,R_A,M_Y},
	/* 20 - 2F */
	{zAND,R_A,R_A},    {zAND,R_A,R_B},     {zAND,R_A,I_8},    {zAND,R_A,M_IHL},
	{zAND,R_A,M_N8},   {zAND,R_A,M_I16},   {zAND,R_A,M_X},    {zAND,R_A,M_Y},
	{zOR,R_A,R_A},     {zOR,R_A,R_B},      {zOR,R_A,I_8},     {zOR,R_A,M_IHL},
	{zOR,R_A,M_N8},    {zOR,R_A,M_I16},    {zOR,R_A,M_X},     {zOR,R_A,M_Y},
	/* 30 - 3F */
	{zCMP,R_A,R_A},    {zCMP,R_A,R_B},     {zCMP,R_A,I_8},    {zCMP,R_A,M_IHL},
	{zCMP,R_A,M_N8},   {zCMP,R_A,M_I16},   {zCMP,R_A,M_X},    {zCMP,R_A,M_Y},
	{zXOR,R_A,R_A},    {zXOR,R_A,R_B},     {zXOR,R_A,I_8},    {zXOR,R_A,M_IHL},
	{zXOR,R_A,M_N8},   {zXOR,R_A,M_I16},   {zXOR,R_A,M_X},    {zXOR,R_A,M_Y},
	/* 40 - 4F */
	{zMOV,R_A,R_A},    {zMOV,R_A,R_B},     {zMOV,R_A,R_L},    {zMOV,R_A,R_H},
	{zMOV,R_A,M_N8},   {zMOV,R_A,M_IHL},   {zMOV,R_A,M_X},    {zMOV,R_A,M_Y},
	{zMOV,R_B,R_A},    {zMOV,R_B,R_B},     {zMOV,R_B,R_L},    {zMOV,R_B,R_H},
	{zMOV,R_B,M_N8},   {zMOV,R_B,M_IHL},   {zMOV,R_B,M_X},    {zMOV,R_B,M_Y},
	/* 50 - 5F */
	{zMOV,R_L,R_A},    {zMOV,R_L,R_B},     {zMOV,R_L,R_L},    {zMOV,R_L,R_H},
	{zMOV,R_L,M_N8},   {zMOV,R_L,M_IHL},   {zMOV,R_L,M_X},    {zMOV,R_L,M_Y},
	{zMOV,R_H,R_A},    {zMOV,R_H,R_B},     {zMOV,R_H,R_L},    {zMOV,R_H,R_H},
	{zMOV,R_H,M_N8},   {zMOV,R_H,M_IHL},   {zMOV,R_H,M_X},    {zMOV,R_H,M_Y},
	/* 60 - 6F */
	{zMOV,M_X,R_A},    {zMOV,M_X,R_B},     {zMOV,M_X,R_L},    {zMOV,M_X,R_H},
	{zMOV,M_X,M_N8},   {zMOV,M_X,M_IHL},   {zMOV,M_X,M_X},    {zMOV,M_X,M_Y},
	{zMOV,M_IHL,R_A},  {zMOV,M_IHL,R_B},   {zMOV,M_IHL,R_L},  {zMOV,M_IHL,R_H},
	{zMOV,M_IHL,M_N8}, {zMOV,M_IHL,M_IHL}, {zMOV,M_IHL,M_X},  {zMOV,M_IHL,M_Y},
	/* 70 - 7F */
	{zMOV,M_Y,R_A},    {zMOV,M_Y,R_B},     {zMOV,M_Y,R_L},    {zMOV,M_Y,R_H},
	{zMOV,M_Y,M_N8},   {zMOV,M_Y,M_IHL},   {zMOV,M_Y,M_X},    {zMOV,M_Y,M_Y},
	{zMOV,M_N8,R_A},   {zMOV,M_N8,R_B},    {zMOV,M_N8,R_L},   {zMOV,M_N8,R_H},
	{zDB,OP,0},        {zMOV,M_N8,M_IHL},  {zMOV,M_N8,M_X},   {zMOV,M_N8,M_Y},
	/* 80 - 8F */
	{zINC,R_A,0},      {zINC,R_B,0},       {zINC,R_L,0},      {zINC,R_H,0},
	{zINC,R_N,0},      {zINC,M_N8,0},      {zINC,M_IHL,0},    {zINC,R_SP,0},
	{zDEC,R_A,0},      {zDEC,R_B,0},       {zDEC,R_L,0},      {zDEC,R_H,0},
	{zDEC,R_N,0},      {zDEC,M_N8,0},      {zDEC,M_IHL,0},    {zDEC,R_SP,0},
	/* 90 - 9F */
	{zINC,R_BA,0},     {zINC,R_HL,0},      {zINC,R_X,0},      {zINC,R_Y,0},
	{zTEST,R_A,R_B},   {zTEST,M_IHL,I_8},  {zTEST,R_A,I_8},   {zTEST,R_B,I_8},
	{zDEC,R_BA,0},     {zDEC,R_HL,0},      {zDEC,R_X,0},      {zDEC,R_Y,0},
	{zAND,R_F,I_8},    {zOR,R_F,I_8},      {zXOR,R_F,I_8},    {zMOV,R_F,I_8},
	/* A0 - AF */
	{zPUSH,R_BA,0},    {zPUSH,R_HL,0},     {zPUSH,R_X,0},     {zPUSH,R_Y,0},
	{zPUSH,R_N,0},     {zPUSH,R_I,0},      {zPUSHX,0,0},      {zPUSH,R_F,0},
	{zPOP,R_BA,0},     {zPOP,R_HL,0},      {zPOP,R_X,0},      {zPOP,R_Y,0},
	{zPOP,R_N,0},      {zPOP,R_I,0},       {zPOP,0,0},        {zPOP,R_F,0},
	/* B0 - BF */
	{zMOV,R_A,I_8},    {zMOV,R_B,I_8},     {zMOV,R_L,I_8},    {zMOV,R_H,I_8},
	{zMOV,R_N,I_8},    {zMOV,M_IHL,I_8},   {zMOV,M_X,I_8},    {zMOV,M_Y,I_8},
	{zMOV,R_BA,M_I16}, {zMOV,R_HL,M_I16},  {zMOV,R_X,M_I16},  {zMOV,R_Y,M_I16},
	{zMOV,M_I16,R_BA}, {zMOV,M_I16,R_HL},  {zMOV,M_I16,R_X},  {zMOV,M_I16,R_Y},
	/* C0 - CF */
	{zADD,R_BA,I_16},  {zADD,R_HL,I_16},   {zADD,R_X,I_16},   {zADD,R_Y,I_16},
	{zMOV,R_BA,I_16},  {zMOV,R_HL,I_16},   {zMOV,R_X,I_16},   {zMOV,R_Y,I_16},
	{zXCHG,R_BA,R_HL}, {zXCHG,R_BA,R_X},   {zXCHG,R_BA,R_Y},  {zXCHG,R_BA,R_SP},
	{zXCHG,R_A,R_B},   {zXCHG,R_A,M_IHL},  {zDB,0,0},         {zDB,0,0},
	/* D0 - DF */
	{zSUB,R_BA,I_16},  {zSUB,R_HL,I_16},   {zSUB,R_X,I_16},   {zSUB,R_Y,I_16},
	{zCMP,R_BA,I_16},  {zCMP,R_HL,I_16},   {zCMP,R_X,I_16},   {zCMP,R_Y,I_16},
	{zAND,M_N8,I_8},   {zOR,M_N8,I_8},     {zXOR,M_N8,I_8},   {zCMP,M_N8,I_8},
	{zTEST,M_N8,I_8},  {zMOV,M_N8,I_8},    {zBCDE,0,0},       {zBCDD,0,0},
	/* E0 - EF */
	{zCALLC,D_8,0},    {zCALLNC,D_8,0},    {zCALLZ,D_8,0},    {zCALLNZ,D_8,0},
	{zJC,D_8,0},       {zJNC,D_8,0},       {zJZ,D_8,0},       {zJNZ,D_8,0},
	{zCALLC,D_16,0},   {zCALLNC,D_16,0},   {zCALLZ,D_16,0},   {zCALLNZ,D_16,0},
	{zJC,D_16,0},      {zJNC,D_16,0},      {zJZ,D_16,0},      {zJNZ,D_16,0},
	/* F0 - FF */
	{zCALL,D_8,0},     {zJMP,D_8,0},       {zCALL,D_16},      {zJMP,D_16},
	{zJMP,R_HL,0},     {zJDBNZ,D_8,0},     {zBCDX,R_A,0},     {zBCDX,M_IHL,0},
	{zRET,0,0},        {zRETI,0,0},        {zRETSKIP,0,0},    {zCALL,M_I16},
	{zINT,I_8,0},      {zJINT,I_8,0},      {zDB,OP,0},        {zNOP,0,0}
};

const minx_disassembler::minxdasm minx_disassembler::mnemonic_ce[256] = {
	/* 00 - 0F */
	{zADD,R_A,M_X8},   {zADD,R_A,M_Y8},    {zADD,R_A,M_XL},   {zADD,R_A,M_YL},
	{zADD,M_IHL,R_A},  {zADD,M_IHL,I_8},   {zADD,M_IHL,M_X},  {zADD,M_IHL,M_Y},
	{zADDC,R_A,M_X8},  {zADDC,R_A,M_Y8},   {zADDC,R_A,M_XL},  {zADDC,R_A,M_YL},
	{zADDC,M_IHL,R_A}, {zADDC,M_IHL,I_8},  {zADDC,M_IHL,M_X}, {zADDC,M_IHL,M_Y},
	/* 10 - 1F */
	{zSUB,R_A,M_X8},   {zSUB,R_A,M_Y8},    {zSUB,R_A,M_XL},   {zSUB,R_A,M_YL},
	{zSUB,M_IHL,R_A},  {zSUB,M_IHL,I_8},   {zSUB,M_IHL,M_X},  {zSUB,M_IHL,M_Y},
	{zSUBC,R_A,M_X8},  {zSUBC,R_A,M_Y8},   {zSUBC,R_A,M_XL},  {zSUBC,R_A,M_YL},
	{zSUBC,M_IHL,R_A}, {zSUBC,M_IHL,I_8},  {zSUBC,M_IHL,M_X}, {zSUBC,M_IHL,M_Y},
	/* 20 - 2F */
	{zAND,R_A,M_X8},   {zAND,R_A,M_Y8},    {zAND,R_A,M_XL},   {zAND,R_A,M_YL},
	{zAND,M_IHL,R_A},  {zAND,M_IHL,I_8},   {zAND,M_IHL,M_X},  {zAND,M_IHL,M_Y},
	{zOR,R_A,M_X8},    {zOR,R_A,M_Y8},     {zOR,R_A,M_XL},    {zOR,R_A,M_YL},
	{zOR,M_IHL,R_A},   {zOR,M_IHL,I_8},    {zOR,M_IHL,M_X},   {zOR,M_IHL,M_Y},
	/* 30 - 3F */
	{zCMP,R_A,M_X8},   {zCMP,R_A,M_Y8},    {zCMP,R_A,M_XL},   {zCMP,R_A,M_YL},
	{zCMP,M_IHL,R_A},  {zCMP,M_IHL,I_8},   {zCMP,M_IHL,M_X},  {zCMP,M_IHL,M_Y},
	{zXOR,R_A,M_X8},   {zXOR,R_A,M_Y8},    {zXOR,R_A,M_XL},   {zXOR,R_A,M_YL},
	{zXOR,M_IHL,R_A},  {zXOR,M_IHL,I_8},   {zXOR,M_IHL,M_X},  {zXOR,M_IHL,M_Y},
	/* 40 - 4F */
	{zMOV,R_A,M_X8},   {zMOV,R_A,M_Y8},    {zMOV,R_A,M_XL},   {zMOV,R_A,M_YL},
	{zMOV,M_X8,R_A},   {zMOV,M_Y8,R_A},    {zMOV,M_X,R_A},    {zMOV,M_Y,R_A},
	{zMOV,R_B,M_X8},   {zMOV,R_B,M_Y8},    {zMOV,R_B,M_XL},   {zMOV,R_B,M_YL},
	{zMOV,M_X8,R_B},   {zMOV,M_Y8,R_B},    {zMOV,M_X,R_B},    {zMOV,M_Y,R_B},
	/* 50 - 5F */
	{zMOV,R_L,M_X8},   {zMOV,R_L,M_Y8},    {zMOV,R_L,M_XL},   {zMOV,R_L,M_YL},
	{zMOV,M_X8,R_L},   {zMOV,M_Y8,R_L},    {zMOV,M_X,R_L},    {zMOV,M_Y,R_L},
	{zMOV,R_H,M_X8},   {zMOV,R_H,M_Y8},    {zMOV,R_H,M_XL},   {zMOV,R_H,M_YL},
	{zMOV,M_X8,R_H},   {zMOV,M_Y8,R_H},    {zMOV,M_X,R_H},    {zMOV,M_Y,R_H},
	/* 60 - 6F */
	{zMOV,M_IHL,M_X8}, {zMOV,M_IHL,M_Y8},  {zMOV,M_IHL,M_XL}, {zMOV,M_IHL,M_YL},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zMOV,M_X,M_X8},   {zMOV,M_X,M_Y8},    {zMOV,M_X,M_XL},   {zMOV,M_X,M_YL},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	/* 70 - 7F */
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zMOV,M_Y,M_X8},   {zMOV,M_Y,M_Y8},    {zMOV,M_Y,M_XL},   {zMOV,M_Y,M_YL},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	/* 80 - 8F */
	{zSAL,R_A,0},      {zSAL,R_B,0},       {zSAL,M_N8,0},     {zSAL,M_IHL,0},
	{zSHL,R_A,0},      {zSHL,R_B,0},       {zSHL,M_N8,0},     {zSHL,M_IHL,0},
	{zSAR,R_A,0},      {zSAR,R_B,0},       {zSAR,M_N8,0},     {zSAR,M_IHL,0},
	{zSHR,R_A,0},      {zSHR,R_B,0},       {zSHR,M_N8,0},     {zSHR,M_IHL,0},
	/* 90 - 9F */
	{zROLC,R_A,0},     {zROLC,R_B,0},      {zROLC,M_N8,0},    {zROLC,M_IHL,0},
	{zROL,R_A,0},      {zROL,R_B,0},       {zROL,M_N8,0},     {zROL,M_IHL,0},
	{zRORC,R_A,0},     {zRORC,R_B,0},      {zRORC,M_N8,0},    {zRORC,M_IHL,0},
	{zROR,R_A,0},      {zROR,R_B,0},       {zROR,M_N8,0},     {zROR,M_IHL,0},
	/* A0 - AF */
	{zNOT,R_A,0},      {zNOT,R_B,0},       {zNOT,M_N8,0},     {zNOT,M_IHL,0},
	{zNEG,R_A,0},      {zNEG,R_B,0},       {zNEG,M_N8,0},     {zNEG,M_IHL,0},
	{zEXT,R_BA,R_A},   {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zHALT,0,0},       {zNOP,0,0},
	/* B0 - BF */
	{zAND,R_B,I_8},    {zAND,R_L,I_8},     {zAND,R_H,I_8},    {zDB,OP1,OP},
	{zOR,R_B,I_8},     {zOR,R_L,I_8},      {zOR,R_H,I_8},     {zDB,OP1,OP},
	{zXOR,R_B,I_8},    {zXOR,R_L,I_8},     {zXOR,R_H,I_8},    {zDB,OP1,OP},
	{zCMP,R_B,I_8},    {zCMP,R_L,I_8},     {zCMP,R_H,I_8},    {zCMP,R_N,I_8},
	/* C0 - CF */
	{zMOV,R_A,R_N},    {zMOV,R_A,R_F},     {zMOV,R_N,R_A},    {zMOV,R_F,R_A},
	{zMOV,R_U,I_8},    {zMOV,R_I,I_8},     {zMOV,R_XI,I_8},   {zMOV,R_YI,I_8},
	{zMOV,R_A,R_V},    {zMOV,R_A,R_I},     {zMOV,R_A,R_XI},   {zMOV,R_A,R_YI},
	{zMOV,R_U,R_A},    {zMOV,R_I,R_A},     {zMOV,R_XI,R_A},   {zMOV,R_YI,R_A},
	/* D0 - DF */
	{zMOV,R_A,M_16},   {zMOV,R_B,M_16},    {zMOV,R_H,M_16},   {zMOV,R_L,M_16},
	{zMOV,M_16,R_A},   {zMOV,M_16,R_B},    {zMOV,M_16,R_H},   {zMOV,M_16,R_L},
	{zMUL,R_HL,R_A},   {zDIV,R_HL,R_A},    {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	/* E0 - EF */
	{zJL,D_8,0},       {zJLE,D_8,0},       {zJG,D_8,0},       {zJGE,D_8,0},
	{zJO,D_8,0},       {zJNO,D_8,0},       {zJP,D_8,0},       {zJN,D_8,0},
	{zJNX0,D_8,0},     {zJNX1,D_8,0},      {zJNX2,D_8,0},     {zJNX3,D_8,0},
	{zJX0,D_8,0},      {zJX1,D_8,0},       {zJX2,D_8,0},      {zJX3,D_8,0},
	/* F0 - FF */
	{zCALLL,D_8,0},    {zCALLLE,D_8,0},    {zCALLG,D_8,0},    {zCALLGE,D_8,0},
	{zCALLO,D_8,0},    {zCALLNO,D_8,0},    {zCALLP,D_8,0},    {zCALLN,D_8,0},
	{zCALLNX0,D_8,0},  {zCALLNX1,D_8,0},   {zCALLNX2,D_8,0},  {zCALLNX3,D_8,0},
	{zCALLX0,D_8,0},   {zCALLX1,D_8,0},    {zCALLX2,D_8,0},   {zCALLX3,D_8,0}
};

const minx_disassembler::minxdasm minx_disassembler::mnemonic_cf[256] = {
	/* 00 - 0F */
	{zADD,R_BA,R_BA},  {zADD,R_BA,R_HL},   {zADD,R_BA,R_X},   {zADD,R_BA,R_Y},
	{zADDC,R_BA,R_BA}, {zADDC,R_BA,R_HL},  {zADDC,R_BA,R_X},  {zADDC,R_BA,R_Y},
	{zSUB,R_BA,R_BA},  {zSUB,R_BA,R_HL},   {zSUB,R_BA,R_X},   {zSUB,R_BA,R_Y},
	{zSUBC,R_BA,R_BA}, {zSUBC,R_BA,R_HL},  {zSUBC,R_BA,R_X},  {zSUBC,R_BA,R_Y},
	/* 10 - 1F */
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zCMP,R_BA,R_BA},  {zCMP,R_BA,R_HL},   {zCMP,R_BA,R_X},   {zCMP,R_BA,R_Y},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	/* 20 - 2F */
	{zADD,R_HL,R_BA},  {zADD,R_HL,R_HL},   {zADD,R_HL,R_X},   {zADD,R_HL,R_Y},
	{zADDC,R_HL,R_BA}, {zADDC,R_HL,R_HL},  {zADDC,R_HL,R_X},  {zADDC,R_HL,R_Y},
	{zSUB,R_HL,R_BA},  {zSUB,R_HL,R_HL},   {zSUB,R_HL,R_X},   {zSUB,R_HL,R_Y},
	{zSUBC,R_HL,R_BA}, {zSUBC,R_HL,R_HL},  {zSUBC,R_HL,R_X},  {zSUBC,R_HL,R_Y},
	/* 30 - 3F */
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zCMP,R_HL,R_BA},  {zCMP,R_HL,R_HL},   {zCMP,R_HL,R_X},   {zCMP,R_HL,R_Y},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	/* 40 - 4F */
	{zADD,R_X,R_BA},   {zADD,R_X,R_HL},    {zADD,R_Y,R_BA},   {zADD,R_Y,R_HL},
	{zADD,R_SP,R_BA},  {zADD,R_SP,R_HL},   {zDB,OP1,},        {zDB,OP1,OP},
	{zSUB,R_X,R_BA},   {zSUB,R_X,R_HL},    {zSUB,R_Y,R_BA},   {zSUB,R_Y,R_HL},
	{zSUB,R_SP,R_BA},  {zSUB,R_SP,R_HL},   {zDB,OP1,OP},      {zDB,OP1,OP},
	/* 50 - 5F */
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zCMP,R_SP,R_BA},  {zCMP,R_SP,R_HL},   {zDB,OP1,OP},      {zDB,OP1,OP},
	/* 60 - 6F */
	{zCMPN,R_BA,I_16}, {zCMPN,R_HL,I_16},  {zCMPN,R_X,I_16},  {zCMPN,R_Y,I_16},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zADD,R_SP,I_16},  {zDB,OP1,OP},       {zSUB,R_SP,I_16},  {zDB,OP1,OP},
	{zCMP,R_SP,I_16},  {zDB,OP1,OP},       {zMOV,R_SP,I_16},  {zDB,OP1,OP},
	/* 70 - 7F */
	{zMOV,R_BA,S_8},   {zMOV,R_HL,S_8},    {zMOV,R_X,S_8},    {zMOV,R_Y,S_8},
	{zMOV,S_8,R_BA},   {zMOV,S_8,R_HL},    {zMOV,S_8,R_X},    {zMOV,S_8,R_Y},
	{zMOV,R_SP,M_I16}, {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zMOV,M_I16,R_SP}, {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	/* 80 - 8F */
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	/* 90 - 9F */
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	/* A0 - AF */
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	/* B0 - BF */
	{zPUSH,R_A,0},     {zPUSH,R_B,0},      {zPUSH,R_L,0},     {zPUSH,R_H,0},
	{zPOP,R_A,0},      {zPOP,R_B,0},       {zPOP,R_L,0},      {zPOP,R_H,0},
	{zPUSHA,0,0},      {zPUSHAX,0,0},      {zDB,OP1,OP},      {zDB,OP1,OP},
	{zPOPA,0,0},       {zPOPAX,0,0},       {zDB,OP1,OP},      {zDB,OP1,OP},
	/* C0 - CF */
	{zMOV,R_BA,M_HL},  {zMOV,R_HL,M_HL},   {zMOV,R_X,M_HL},   {zMOV,R_Y,M_HL},
	{zMOV,M_HL,R_BA},  {zMOV,M_HL,R_HL},   {zMOV,M_HL,R_X},   {zMOV,M_HL,R_Y},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zDB,OP1,OP},      {zDB,OP1,OP},
	/* D0 - DF */
	{zMOV,R_BA,M_X},   {zMOV,R_HL,M_X},    {zMOV,R_X,M_X},    {zMOV,R_Y,M_X},
	{zMOV,M_X,R_BA},   {zMOV,M_X,R_HL},    {zMOV,M_X,R_X},    {zMOV,M_X,R_Y},
	{zMOV,R_BA,M_Y},   {zMOV,R_HL,M_Y},    {zMOV,R_X,M_Y},    {zMOV,R_Y,M_Y},
	{zMOV,M_Y,R_BA},   {zMOV,M_Y,R_HL},    {zMOV,M_Y,R_X},    {zMOV,M_Y,R_Y},
	/* E0 - EF */
	{zMOV,R_BA,R_BA},  {zMOV,R_BA,R_HL},   {zMOV,R_BA,R_X},   {zMOV,R_BA,R_Y},
	{zMOV,R_HL,R_BA},  {zMOV,R_HL,R_HL},   {zMOV,R_HL,R_X},   {zMOV,R_HL,R_Y},
	{zMOV,R_X,R_BA},   {zMOV,R_X,R_HL},    {zMOV,R_X,R_X},    {zMOV,R_X,R_Y},
	{zMOV,R_Y,R_BA},   {zMOV,R_Y,R_HL},    {zMOV,R_Y,R_X},    {zMOV,R_Y,R_Y},
	/* F0 - FF */
	{zMOV,R_SP,R_BA},  {zMOV,R_SP,R_HL},   {zMOV,R_SP,R_X},   {zMOV,R_SP,R_Y},
	{zMOV,R_HL,R_SP},  {zMOV,R_HL,R_PC},   {zDB,OP1,OP},      {zDB,OP1,OP},
	{zMOV,R_BA,R_SP},  {zMOV,R_BA,R_PC},   {zMOV,R_X,R_SP},   {zDB,OP1,OP},
	{zDB,OP1,OP},      {zDB,OP1,OP},       {zMOV,R_Y,R_SP},   {zDB,OP1,OP}
};

#define HANDLE_ARGUMENT                         \
case R_A:   util::stream_format(stream, "%cA", fill); break;          \
case R_B:   util::stream_format(stream, "%cB", fill); break;          \
case R_L:   util::stream_format(stream, "%cL", fill); break;          \
case R_H:   util::stream_format(stream, "%cH", fill); break;          \
case R_N:   util::stream_format(stream, "%cN", fill); break;          \
case R_F:   util::stream_format(stream, "%cF", fill); break;          \
case R_SP:  util::stream_format(stream, "%cSP", fill); break;         \
case R_BA:  util::stream_format(stream, "%cBA", fill); break;         \
case R_HL:  util::stream_format(stream, "%cHL", fill); break;         \
case R_X:   util::stream_format(stream, "%cX", fill); break;          \
case R_Y:   util::stream_format(stream, "%cY", fill); break;          \
case R_U:   util::stream_format(stream, "%cU", fill); break;          \
case R_V:   util::stream_format(stream, "%cV", fill); break;          \
case R_I:   util::stream_format(stream, "%cI", fill); break;          \
case R_XI:  util::stream_format(stream, "%cXI", fill); break;         \
case R_YI:  util::stream_format(stream, "%cYI", fill); break;         \
case R_PC:  util::stream_format(stream, "%cPC", fill); break;         \
case I_8:            /* 8 bit immediate */              \
	ea = opcodes.r8(pos++);                      \
	util::stream_format(stream, "%c$%02X", fill, ea);         \
	break;                              \
case I_16:           /* 16 bit immediate */             \
	ea = opcodes.r8(pos++);                      \
	ea += opcodes.r8(pos++) << 8;                    \
	util::stream_format(stream, "%c$%04X", fill, ea);         \
	break;                              \
case D_8:            /* PC + 8 bit displacement (signed) */     \
	ofs8 = opcodes.r8(pos++);                        \
	util::stream_format(stream, "%c$%04X", fill, pos - 1 + ofs8);    \
	break;                              \
case D_16:           /* PC + 16 bit displacement */         \
	ea = opcodes.r8(pos++);                      \
	ea += opcodes.r8(pos++) << 8;                    \
	ea = ea - 1;                            \
	util::stream_format(stream, "%c$%04X", fill, pos + ea);      \
	break;                              \
case S_8:            /* SP + 8 bit displacement (signed) */     \
	ea = opcodes.r8(pos++);                      \
	util::stream_format(stream, "%cSP+$%02X", fill, ea);          \
	break;                              \
case M_IHL: util::stream_format(stream, "%c[I+HL]", fill); break;     \
case M_N8:           /* [I+N+ofs8] */                   \
	ea = opcodes.r8(pos++);                      \
	util::stream_format(stream, "%c[I+N+$%02X]", fill, ea);       \
	break;                              \
case M_I16:          /* [I+ofs16] */                    \
	ea = opcodes.r8(pos++);                      \
	ea += opcodes.r8(pos++) << 8;                    \
	util::stream_format(stream, "%c[I+$%04X]", fill, ea);         \
	break;                              \
case M_X:   util::stream_format(stream, "%c[X]", fill); break;        \
case M_Y:   util::stream_format(stream, "%c[Y]", fill); break;        \
case M_X8:           /* [X + 8 bit displacement (signed)] */        \
	ea = opcodes.r8(pos++);                      \
	util::stream_format(stream, "%c[X+$%02X]", fill, ea);         \
	break;                              \
case M_Y8:           /* [Y + 8 bit displacement (signed)] */        \
	ea = opcodes.r8(pos++);                      \
	util::stream_format(stream, "%c[Y+$%02X]", fill, ea);         \
	break;                              \
case M_XL:  util::stream_format(stream, "%c[X+L]", fill); break;      \
case M_YL:  util::stream_format(stream, "%c[Y+L]", fill); break;      \
case M_16:           /* [16bit] */                  \
	ea = opcodes.r8(pos++);                      \
	ea += opcodes.r8(pos++) << 8;                    \
	util::stream_format(stream, "%c[$%04X]", fill, ea);           \
	break;                              \
case M_HL:  util::stream_format(stream, "%c[HL]", fill); break;       \
case OP:    util::stream_format(stream, "%c$%02X", fill, op); break;      \
case OP1:   util::stream_format(stream, "%c$%02X", fill, op1); break;

u32 minx_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t minx_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const minxdasm *instr;
	uint8_t op, op1;
	int8_t  ofs8;
	uint16_t ea;
	offs_t pos = pc;

	op1 = op = opcodes.r8(pos++);

	switch (op) {
	case 0xCE:
		op = opcodes.r8(pos++);
		instr = &mnemonic_ce[op];
		break;
	case 0xCF:
		op = opcodes.r8(pos++);
		instr = &mnemonic_cf[op];
		break;
	default:
		instr = &mnemonic[op];
		break;
	}

	util::stream_format(stream, "%-6s", s_mnemonic[ instr->mnemonic ]);

	if ( instr->argument1 ) {
		char fill = ' ';
		//int arg = 0;
		switch( instr->argument1 ) {
			HANDLE_ARGUMENT;
		}
	}
	if ( instr->argument2 ) {
		char fill = ',';
		//int arg = 1;
		switch( instr->argument2 ) {
			HANDLE_ARGUMENT;
		}
	}
	return (pos - pc) | s_flags[instr->mnemonic] | SUPPORTED;
}
