// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 *   r800dasm.cpp
 *   Disassembler for ASCII R800 based on portable Z80 disassembler.
 *
 *****************************************************************************/

#include "emu.h"
#include "r800dasm.h"

enum r800_disassembler::e_mnemonics : unsigned
{
	zADD  ,zADDC ,zADJ  ,zAND  ,zBC   ,zBIT  ,zBM   ,zBNC  ,
	zBNZ  ,zBP   ,zBPE  ,zBPO  ,zBR   ,zBRK  ,zBZ   ,zCALL ,
	zCLR  ,zCMP  ,zCMPM ,zDB   ,zDBNZ ,zDEC  ,zDI   ,zEI   ,
	zHALT ,zIM   ,zIN   ,zINC  ,zINM  ,zLD   ,zMOVE ,zMOVEM,
	zMULUB,zMULUW,zNEG  ,zNOP  ,zNOT  ,zNOTC ,zOR   ,zOUT  ,
	zOUTM ,zPOP  ,zPUSH ,zRET  ,zRETI ,zRETN ,zROL  ,zROL4 ,
	zROLA ,zROLC ,zROLCA,zROR  ,zROR4 ,zRORA ,zRORC ,zRORCA,
	zSET  ,zSETC ,zSHL  ,zSHORT,zSHR  ,zSHRA ,zSLA  ,zSUB  ,
	zSUBC ,zXCH  ,zXCHX ,zXOR
};

struct r800_disassembler::r800dasm
{
	e_mnemonics mnemonic;
	const char *arguments;
};

static const char *const s_mnemonic[] =
{
	"add"  ,"addc" ,"adj"  ,"and"  ,"bc"   ,"bit"  ,"bm"   ,"bnc"  ,
	"bnz"  ,"bp"   ,"bpe"  ,"bpo"  ,"br"   ,"brk"  ,"bz"   ,"call" ,
	"clr"  ,"cmp"  ,"cmpm" ,"db"   ,"dbnz" ,"dec"  ,"di"   ,"ei"   ,
	"halt" ,"im"   ,"in"   ,"inc"  ,"inm"  ,"ld"   ,"move" ,"movem",
	"mulub","muluw","neg"  ,"nop"  ,"not"  ,"notc" ,"or"   ,"out"  ,
	"outm" ,"pop"  ,"push" ,"ret"  ,"reti" ,"retn" ,"rol"  ,"rol4" ,
	"rola" ,"rolc" ,"rolca","ror"  ,"ror4" ,"rora" ,"rorc" ,"rorca",
	"set"  ,"setc" ,"shl"  ,"short","shr"  ,"shra" ,"sla"  ,"sub"  ,
	"subc" ,"xch"  ,"xchx" ,"xor"
};

const u32 r800_disassembler::s_flags[] =
{
	0        ,0        ,0        ,0        ,0        ,0        ,0        ,0        ,
	0        ,0        ,0        ,0        ,0        ,STEP_OVER,0        ,STEP_OVER,
	0        ,0        ,STEP_COND,0        ,STEP_COND,0        ,0        ,0        ,
	STEP_OVER,0        ,0        ,0        ,STEP_COND,0        ,0        ,STEP_COND,
	0        ,0        ,0        ,0        ,0        ,0        ,0        ,0        ,
	STEP_COND,0        ,0        ,STEP_OUT ,STEP_OUT ,STEP_OUT ,0        ,0        ,
	0        ,0        ,0        ,0        ,0        ,0        ,0        ,0        ,
	0        ,0        ,0        ,0        ,0        ,0        ,0        ,0        ,
	0        ,0        ,0        ,0
};

const r800_disassembler::r800dasm r800_disassembler::mnemonic_xx_cb[256] =
{
	// 00 - 0F
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zROLC,"Y"},   {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zRORC,"Y"},   {zDB,"?"},
	// 10 - 1F
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zROL,"Y"},    {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zROR,"Y"},    {zDB,"?"},
	// 20 - 2F
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zSLA,"Y"},    {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zSHRA,"Y"},   {zDB,"?"},
	// 30 - 3F
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zSHL,"Y"},    {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zSHR,"Y"},    {zDB,"?"},
	// 40 - 4F
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zBIT,"0,Y"},  {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zBIT,"1,Y"},  {zDB,"?"},
	// 50 - 5F
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zBIT,"2,Y"},  {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zBIT,"3,Y"},  {zDB,"?"},
	// 60 - 6F
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zBIT,"4,Y"},  {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zBIT,"5,Y"},  {zDB,"?"},
	// 70 - 6F
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zBIT,"6,Y"},  {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zBIT,"7,Y"},  {zDB,"?"},
	// 80 - 8F
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zCLR,"0,Y"},  {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zCLR,"1,Y"},  {zDB,"?"},
	// 90 - 9F
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zCLR,"2,Y"},  {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zCLR,"3,Y"},  {zDB,"?"},
	// A0 - AF
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zCLR,"4,Y"},  {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zCLR,"5,Y"},  {zDB,"?"},
	// B0 - BF
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zCLR,"6,Y"},  {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zCLR,"7,Y"},  {zDB,"?"},
	// C0 - CF
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zSET,"0,Y"},  {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zSET,"1,Y"},  {zDB,"?"},
	// D0 - DF
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zSET,"2,Y"},  {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zSET,"3,Y"},  {zDB,"?"},
	// E0 - EF
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zSET,"4,Y"},  {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zSET,"5,Y"},  {zDB,"?"},
	// F0 - FF
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zSET,"6,Y"},  {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zDB,"?"},     {zDB,"?"},
	{zDB,"?"},  {zDB,"?"},  {zSET,"7,Y"},  {zDB,"?"}
};

const r800_disassembler::r800dasm r800_disassembler::mnemonic_cb[256] =
{
	// 00 - 1F
	{zROLC,".b"},   {zROLC,".c"},   {zROLC,".d"},      {zROLC,".e"},
	{zROLC,".h"},   {zROLC,".l"},   {zROLC,"[.hl]"},   {zROLC,".a"},
	{zRORC,".b"},   {zRORC,".c"},   {zRORC,".d"},      {zRORC,".e"},
	{zRORC,".h"},   {zRORC,".l"},   {zRORC,"[.hl]"},   {zRORC,".a"},
	// 10 - 1F
	{zROL,".b"},    {zROL,".c"},    {zROL,".d"},       {zROL,".e"},
	{zROL,".h"},    {zROL,".l"},    {zROL,"[.hl]"},    {zROL,".a"},
	{zROR,".b"},    {zROR,".c"},    {zROR,".d"},       {zROR,".e"},
	{zROR,".h"},    {zROR,".l"},    {zROR,"[.hl]"},    {zROR,".a"},
	// 20 - 2F
	{zSHL,".b"},    {zSHL,".c"},    {zSHL,".d"},       {zSHL,".e"},
	{zSHL,".h"},    {zSHL,".l"},    {zSHL,"[.hl]"},    {zSHL,".a"},
	{zSHRA,".b"},   {zSHRA,".c"},   {zSHRA,".d"},      {zSHRA,".e"},
	{zSHRA,".h"},   {zSHRA,".l"},   {zSHRA,"[.hl]"},   {zSHRA,".a"},
	// 30 - 3F
	{zSHL,".b"},    {zSHL,".c"},    {zSHL,".d"},       {zSHL,".e"},
	{zSHL,".h"},    {zSHL,".l"},    {zSHL,"[.hl]"},    {zSHL,".a"},
	{zSHR,".b"},    {zSHR,".c"},    {zSHR,".d"},       {zSHR,".e"},
	{zSHR,".h"},    {zSHR,".l"},    {zSHR,"[.hl]"},    {zSHR,".a"},
	// 40 - 4F
	{zBIT,"0,.b"},  {zBIT,"0,.c"},  {zBIT,"0,.d"},     {zBIT,"0,.e"},
	{zBIT,"0,.h"},  {zBIT,"0,.l"},  {zBIT,"0,[.hl]"},  {zBIT,"0,.a"},
	{zBIT,"1,.b"},  {zBIT,"1,.c"},  {zBIT,"1,.d"},     {zBIT,"1,.e"},
	{zBIT,"1,.h"},  {zBIT,"1,.l"},  {zBIT,"1,[.hl]"},  {zBIT,"1,.a"},
	// 50 - 5F
	{zBIT,"2,.b"},  {zBIT,"2,.c"},  {zBIT,"2,.d"},     {zBIT,"2,.e"},
	{zBIT,"2,.h"},  {zBIT,"2,.l"},  {zBIT,"2,[.hl]"},  {zBIT,"2,.a"},
	{zBIT,"3,.b"},  {zBIT,"3,.c"},  {zBIT,"3,.d"},     {zBIT,"3,.e"},
	{zBIT,"3,.h"},  {zBIT,"3,.l"},  {zBIT,"3,[.hl]"},  {zBIT,"3,.a"},
	// 60 - 6F
	{zBIT,"4,.b"},  {zBIT,"4,.c"},  {zBIT,"4,.d"},     {zBIT,"4,.e"},
	{zBIT,"4,.h"},  {zBIT,"4,.l"},  {zBIT,"4,[.hl]"},  {zBIT,"4,.a"},
	{zBIT,"5,.b"},  {zBIT,"5,.c"},  {zBIT,"5,.d"},     {zBIT,"5,.e"},
	{zBIT,"5,.h"},  {zBIT,"5,.l"},  {zBIT,"5,[.hl]"},  {zBIT,"5,.a"},
	// 70 - 7F
	{zBIT,"6,.b"},  {zBIT,"6,.c"},  {zBIT,"6,.d"},     {zBIT,"6,.e"},
	{zBIT,"6,.h"},  {zBIT,"6,.l"},  {zBIT,"6,[.hl]"},  {zBIT,"6,.a"},
	{zBIT,"7,.b"},  {zBIT,"7,.c"},  {zBIT,"7,.d"},     {zBIT,"7,.e"},
	{zBIT,"7,.h"},  {zBIT,"7,.l"},  {zBIT,"7,[.hl]"},  {zBIT,"7,.a"},
	// 80 - 8F
	{zCLR,"0,.b"},  {zCLR,"0,.c"},  {zCLR,"0,.d"},     {zCLR,"0,.e"},
	{zCLR,"0,.h"},  {zCLR,"0,.l"},  {zCLR,"0,[.hl]"},  {zCLR,"0,.a"},
	{zCLR,"1,.b"},  {zCLR,"1,.c"},  {zCLR,"1,.d"},     {zCLR,"1,.e"},
	{zCLR,"1,.h"},  {zCLR,"1,.l"},  {zCLR,"1,[.hl]"},  {zCLR,"1,.a"},
	// 90 - 9F
	{zCLR,"2,.b"},  {zCLR,"2,.c"},  {zCLR,"2,.d"},     {zCLR,"2,.e"},
	{zCLR,"2,.h"},  {zCLR,"2,.l"},  {zCLR,"2,[.hl]"},  {zCLR,"2,.a"},
	{zCLR,"3,.b"},  {zCLR,"3,.c"},  {zCLR,"3,.d"},     {zCLR,"3,.e"},
	{zCLR,"3,.h"},  {zCLR,"3,.l"},  {zCLR,"3,[.hl]"},  {zCLR,"3,.a"},
	// A0 - AF
	{zCLR,"4,.b"},  {zCLR,"4,.c"},  {zCLR,"4,.d"},     {zCLR,"4,.e"},
	{zCLR,"4,.h"},  {zCLR,"4,.l"},  {zCLR,"4,[.hl]"},  {zCLR,"4,.a"},
	{zCLR,"5,.b"},  {zCLR,"5,.c"},  {zCLR,"5,.d"},     {zCLR,"5,.e"},
	{zCLR,"5,.h"},  {zCLR,"5,.l"},  {zCLR,"5,[.hl]"},  {zCLR,"5,.a"},
	// B0 - BF
	{zCLR,"6,.b"},  {zCLR,"6,.c"},  {zCLR,"6,.d"},     {zCLR,"6,.e"},
	{zCLR,"6,.h"},  {zCLR,"6,.l"},  {zCLR,"6,[.hl]"},  {zCLR,"6,.a"},
	{zCLR,"7,.b"},  {zCLR,"7,.c"},  {zCLR,"7,.d"},     {zCLR,"7,.e"},
	{zCLR,"7,.h"},  {zCLR,"7,.l"},  {zCLR,"7,[.hl]"},  {zCLR,"7,.a"},
	// C0 - CF
	{zSET,"0,.b"},  {zSET,"0,.c"},  {zSET,"0,.d"},     {zSET,"0,.e"},
	{zSET,"0,.h"},  {zSET,"0,.l"},  {zSET,"0,[.hl]"},  {zSET,"0,.a"},
	{zSET,"1,.b"},  {zSET,"1,.c"},  {zSET,"1,.d"},     {zSET,"1,.e"},
	{zSET,"1,.h"},  {zSET,"1,.l"},  {zSET,"1,[.hl]"},  {zSET,"1,.a"},
	// D0 - DF
	{zSET,"2,.b"},  {zSET,"2,.c"},  {zSET,"2,.d"},     {zSET,"2,.e"},
	{zSET,"2,.h"},  {zSET,"2,.l"},  {zSET,"2,[.hl]"},  {zSET,"2,.a"},
	{zSET,"3,.b"},  {zSET,"3,.c"},  {zSET,"3,.d"},     {zSET,"3,.e"},
	{zSET,"3,.h"},  {zSET,"3,.l"},  {zSET,"3,[.hl]"},  {zSET,"3,.a"},
	// E0 - EF
	{zSET,"4,.b"},  {zSET,"4,.c"},  {zSET,"4,.d"},     {zSET,"4,.e"},
	{zSET,"4,.h"},  {zSET,"4,.l"},  {zSET,"4,[.hl]"},  {zSET,"4,.a"},
	{zSET,"5,.b"},  {zSET,"5,.c"},  {zSET,"5,.d"},     {zSET,"5,.e"},
	{zSET,"5,.h"},  {zSET,"5,.l"},  {zSET,"5,[.hl]"},  {zSET,"5,.a"},
	// F0 - FF
	{zSET,"6,.b"},  {zSET,"6,.c"},  {zSET,"6,.d"},     {zSET,"6,.e"},
	{zSET,"6,.h"},  {zSET,"6,.l"},  {zSET,"6,[.hl]"},  {zSET,"6,.a"},
	{zSET,"7,.b"},  {zSET,"7,.c"},  {zSET,"7,.d"},     {zSET,"7,.e"},
	{zSET,"7,.h"},  {zSET,"7,.l"},  {zSET,"7,[.hl]"},  {zSET,"7,.a"}
};

const r800_disassembler::r800dasm r800_disassembler::mnemonic_ed[256] =
{
	// 00 - 0F
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	// 10 - 1F
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	// 20 - 2F
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	// 30 - 3F
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	// 40 - 4F
	{zIN,".b,[.c]"},            {zOUT,"[.c],.b"},     {zSUBC,".hl,.bc"},     {zLD,"(W),.bc"},
	{zNEG,".a"},                {zRETN,nullptr},      {zIM,"0"},             {zLD,".i,.a"},
	{zIN,".c,[.c]"},            {zOUT,"[.c],.c"},     {zADDC,".hl,.bc"},     {zLD,".bc,(W)"},
	{zDB,"?"},                  {zRETI,nullptr},      {zDB,"?"},             {zLD,".r,.a"},
	// 50 - 5F
	{zIN,".d,[.c]"},            {zOUT,"[.c],.d"},     {zSUBC,".hl,.de"},     {zLD,"(W),.de"},
	{zDB,"?"},                  {zDB,"?"},            {zIM,"1"},             {zLD,".a,.i"},
	{zIN,".e,[.c]"},            {zOUT,"[.c],.e"},     {zADDC,".hl,.de"},     {zLD,".de,(W)"},
	{zDB,"?"},                  {zDB,"?"},            {zIM,"2"},             {zLD,".a,.r"},
	// 60 - 6F
	{zIN,".h,[.c]"},            {zOUT,"[.c],.h"},     {zSUBC,".hl,.hl"},     {zLD,"(W),.hl"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zROR4,"[.hl]"},
	{zIN,".l,[.c]"},            {zOUT,"[.c],.l"},     {zADDC,".hl,.hl"},     {zLD,".hl,(W)"},
	{zDB,"?"},                  {zRETI,nullptr},      {zIM,"0"},             {zROL4,"[.hl]"},
	// 70 - 7F
	{zIN,".f,[.c]"},            {zDB,"?"},            {zSUBC,".hl,.sp"},     {zLD,"(W),.sp"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zIN,".a,[.c]"},            {zOUT,"[.c],.a"},     {zADDC,".hl,.sp"},     {zLD,".sp,(W)"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	// 80 - 8F
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	// 90 - 9F
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	// A0 - AF
	{zMOVE,"[.hl++],[.de++]"},  {zCMP,".a,[.hl++]"},  {zIN,"[.c],[.hl++]"},  {zOUT,"[.c],[.hl++]"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zMOVE,"[.hl--],[.de--]"},  {zCMP,".a,[.hl--]"},  {zIN,"[.c],[.hl--]"},  {zOUT,"[.c],[.hl--]"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	// B0 - BF
	{zMOVEM,"[.hl++],[.de++]"}, {zCMPM,".a,[.hl++]"}, {zINM,"[.c],[.hl++]"}, {zOUTM,"[.c],[.hl++]"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zMOVEM,"[.hl--],[.de--]"}, {zCMPM,".a,[.hl--]"}, {zINM,"[.c],[.hl--]"}, {zOUTM,"[.c],[.hl--]"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	// C0 - CF
	{zDB,"?"},                  {zMULUB,".a,.b"},     {zDB,"?"},             {zMULUW,".hl,.bc"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zMULUB,".a,.c"},     {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	// D0 - DF
	{zDB,"?"},                  {zMULUB,".a,.d"},     {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zMULUB,".a,.e"},     {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	// E0 - EF
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	// F0 - FF
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zMULUW,".hl,.sp"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"},
	{zDB,"?"},                  {zDB,"?"},            {zDB,"?"},             {zDB,"?"}
};

const r800_disassembler::r800dasm r800_disassembler::mnemonic_xx[256] =
{
	// 00 - 0F
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zADD,".I,.bc"},   {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	// 10 - 1F
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zADD,".I,.de"},   {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	// 20 - 2F
	{zDB,"?"},         {zLD,".I,N"},      {zLD,"(W),.I"},  {zINC,".I"},
	{zINC,".Ih"},      {zDEC,".Ih"},      {zLD,".Ih,B"},   {zDB,"?"},
	{zDB,"?"},         {zADD,".I,.I"},    {zLD,".I,(W)"},  {zDEC,".I"},
	{zINC,".Il"},      {zDEC,".Il"},      {zLD,".Il,B"},   {zDB,"?"},
	// 30 - 3F
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zINC,"X"},        {zDEC,"X"},        {zLD,"X,B"},     {zDB,"?"},
	{zDB,"?"},         {zADD,".I,.sp"},   {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	// 40 - 4F
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zLD,".b,.Ih"},    {zLD,".b,.Il"},    {zLD,".b,X"},    {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zLD,".c,.Ih"},    {zLD,".c,.Il"},    {zLD,".c,X"},    {zDB,"?"},
	// 50 - 5F
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zLD,".d,.Ih"},    {zLD,".d,.Il"},    {zLD,".d,X"},    {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zLD,".e,.Ih"},    {zLD,".e,.Il"},    {zLD,".e,X"},    {zDB,"?"},
	// 60 - 6F
	{zLD,".Ih,.b"},    {zLD,".Ih,.c"},    {zLD,".Ih,.d"},  {zLD,".Ih,.e"},
	{zLD,".Ih,.Ih"},   {zLD,".Ih,.Il"},   {zLD,".h,X"},    {zLD,".Ih,.a"},
	{zLD,".Il,.b"},    {zLD,".Il,.c"},    {zLD,".Il,.d"},  {zLD,".Il,.e"},
	{zLD,".Il,.Ih"},   {zLD,".Il,.Il"},   {zLD,".l,X"},    {zLD,".Il,.a"},
	// 70 - 7F
	{zLD,"X,.b"},      {zLD,"X,.c"},      {zLD,"X,.d"},    {zLD,"X,.e"},
	{zLD,"X,.h"},      {zLD,"X,.l"},      {zDB,"?"},       {zLD,"X,.a"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zLD,".a,.Ih"},    {zLD,".a,Il"},     {zLD,".a,X"},    {zDB,"?"},
	// 80 - 8F
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zADD,".a,.Ih"},   {zADD,".a,.Il"},   {zADD,".a,X"},   {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zADDC,".a,.Ih"},  {zADDC,".a,.Il"},  {zADDC,".a,X"},  {zDB,"?"},
	// 90 - 9F
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zSUB,".a,.Ih"},   {zSUB,".a,.Il"},   {zSUB,".a,X"},   {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zSUBC,".a,.Ih"},  {zSUBC,".a,.Il"},  {zSUBC,".a,X"},  {zDB,"?"},
	// A0 - AF
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zAND,".a,.Ih"},   {zAND,".a,.Il"},   {zAND,".a,X"},   {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zXOR,".a,.Ih"},   {zXOR,".a,.Il"},   {zXOR,".a,X"},   {zDB,"?"},
	// B0 - BF
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zOR,".a,.Ih"},    {zOR,".a,.Il"},    {zOR,".a,X"},    {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zCMP,".a,.Ih"},   {zCMP,".a,.Il"},   {zCMP,".a,X"},   {zDB,"?"},
	// C0 - CF
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"cb"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	// D0 - DF
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	// E0 - EF
	{zDB,"?"},         {zPOP,".I"},       {zDB,"?"},       {zXCH,"[.sp],.I"},
	{zDB,"?"},         {zPUSH,".I"},      {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zBR,"[.I]"},      {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	// F0 - FF
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zLD,".sp,I"},     {zDB,"?"},       {zDB,"?"},
	{zDB,"?"},         {zDB,"?"},         {zDB,"?"},       {zDB,"?"}
};

const r800_disassembler::r800dasm r800_disassembler::mnemonic_main[256] =
{
	// 00 - 0F
	{zNOP,nullptr},    {zLD,".bc,N"},     {zLD,"[.bc],.a"},    {zINC,".bc"},
	{zINC,".b"},       {zDEC,".b"},       {zLD,".b,B"},        {zROLCA,nullptr},
	{zXCH,".af,.af'"}, {zADD,".hl,.bc"},  {zLD,".a,[.bc]"},    {zDEC,".bc"},
	{zINC,".c"},       {zDEC,".c"},       {zLD,".c,B"},        {zRORCA,nullptr},
	// 10 - 1F
	{zDBNZ,"O"},       {zLD,".de,N"},     {zLD,"[.de],.a"},    {zINC,".de"},
	{zINC,".d"},       {zDEC,".d"},       {zLD,".d,B"},        {zROLA,nullptr},
	{zSHORT,"br O"},   {zADD,".hl,.de"},  {zLD,".a,[.de]"},    {zDEC,".de"},
	{zINC,".e"},       {zDEC,".e"},       {zLD,".e,B"},        {zRORA,nullptr},
	// 20 - 2F
	{zSHORT,"bnz O"},  {zLD,".hl,N"},     {zLD,"(W),.hl"},     {zINC,".hl"},
	{zINC,".h"},       {zDEC,".h"},       {zLD,".h,B"},        {zADJ,".a"},
	{zSHORT,"bz O"},   {zADD,".hl,.hl"},  {zLD,".hl,(W)"},     {zDEC,".hl"},
	{zINC,".l"},       {zDEC,".l"},       {zLD,".l,B"},        {zNOT,".a"},
	// 30 - 3F
	{zSHORT,"bnc O"},  {zLD,".sp,N"},     {zLD,"(W),.a"},      {zINC,".sp"},
	{zINC,"[.hl]"},    {zDEC,"[.hl]"},    {zLD,"[.hl],B"},     {zSETC,nullptr},
	{zSHORT,"bc O"},   {zADD,".hl,.sp"},  {zLD,".a,(W)"},      {zDEC,".sp"},
	{zINC,".a"},       {zDEC,".a"},       {zLD,".a,B"},        {zNOTC,nullptr},
	// 40 - 4F
	{zLD,".b,.b"},     {zLD,".b,.c"},     {zLD,".b,.d"},       {zLD,".b,.e"},
	{zLD,".b,.h"},     {zLD,".b,.l"},     {zLD,".b,[.hl]"},    {zLD,".b,.a"},
	{zLD,".c,.b"},     {zLD,".c,.c"},     {zLD,".c,.d"},       {zLD,".c,.e"},
	{zLD,".c,.h"},     {zLD,".c,.l"},     {zLD,".c,[.hl]"},    {zLD,".c,.a"},
	// 50 - 5F
	{zLD,".d,.b"},     {zLD,".d,.c"},     {zLD,".d,.d"},       {zLD,".d,.e"},
	{zLD,".d,.h"},     {zLD,".d,.l"},     {zLD,".d,[.hl]"},    {zLD,".d,.a"},
	{zLD,".e,.b"},     {zLD,".e,.c"},     {zLD,".e,.d"},       {zLD,".e,.e"},
	{zLD,".e,.h"},     {zLD,".e,.l"},     {zLD,".e,[.hl]"},    {zLD,".e,.a"},
	// 60 - 6F
	{zLD,".h,.b"},     {zLD,".h,.c"},     {zLD,".h,.d"},       {zLD,".h,.e"},
	{zLD,".h,.h"},     {zLD,".h,.l"},     {zLD,".h,[.hl]"},    {zLD,".h,.a"},
	{zLD,".l,.b"},     {zLD,".l,.c"},     {zLD,".l,.d"},       {zLD,".l,.e"},
	{zLD,".l,.h"},     {zLD,".l,.l"},     {zLD,".l,[.hl]"},    {zLD,".l,.a"},
	// 70 - 7F
	{zLD,"[.hl],.b"},  {zLD,"[.hl],.c"},  {zLD,"[.hl],.d"},    {zLD,"[.hl],.e"},
	{zLD,"[.hl],.h"},  {zLD,"[.hl],.l"},  {zHALT,nullptr},     {zLD,"[.hl],.a"},
	{zLD,".a,.b"},     {zLD,".a,.c"},     {zLD,".a,.d"},       {zLD,".a,.e"},
	{zLD,".a,.h"},     {zLD,".a,.l"},     {zLD,".a,[.hl]"},    {zLD,".a,.a"},
	// 80 - BF
	{zADD,".a,.b"},    {zADD,".a,.c"},    {zADD,".a,.d"},      {zADD,".a,.e"},
	{zADD,".a,.h"},    {zADD,".a,.l"},    {zADD,".a,[.hl]"},   {zADD,".a,.a"},
	{zADDC,".a,.b"},   {zADDC,".a,.c"},   {zADDC,".a,.d"},     {zADDC,".a,.e"},
	{zADDC,".a,.h"},   {zADDC,".a,.l"},   {zADDC,".a,[.hl]"},  {zADDC,".a,.a"},
	// 90 - 9F
	{zSUB,".a,.b"},    {zSUB,".a,.c"},    {zSUB,".a,.d"},      {zSUB,".a,.e"},
	{zSUB,".a,.h"},    {zSUB,".a,.l"},    {zSUB,".a,[.hl]"},   {zSUB,".a,.a"},
	{zSUBC,".a,.b"},   {zSUBC,".a,.c"},   {zSUBC,".a,.d"},     {zSUBC,".a,.e"},
	{zSUBC,".a,.h"},   {zSUBC,".a,.l"},   {zSUBC,".a,[.hl]"},  {zSUBC,".a,.a"},
	// A0 - AF
	{zAND,".a,.b"},    {zAND,".a,.c"},    {zAND,".a,.d"},      {zAND,".a,.e"},
	{zAND,".a,.h"},    {zAND,".a,.l"},    {zAND,".a,[.hl]"},   {zAND,".a,.a"},
	{zXOR,".a,.b"},    {zXOR,".a,.c"},    {zXOR,".a,.d"},      {zXOR,".a,.e"},
	{zXOR,".a,.h"},    {zXOR,".a,.l"},    {zXOR,".a,[.hl]"},   {zXOR,".a,.a"},
	// B0 - BF
	{zOR,".a,.b"},     {zOR,".a,.c"},     {zOR,".a,.d"},       {zOR,".a,.e"},
	{zOR,".a,.h"},     {zOR,".a,.l"},     {zOR,".a,[.hl]"},    {zOR,".a,.a"},
	{zCMP,".a,.b"},    {zCMP,".a,.c"},    {zCMP,".d"},         {zCMP,".a,.e"},
	{zCMP,".a,.h"},    {zCMP,".a,.l"},    {zCMP,".a,[.hl]"},   {zCMP,".a,.a"},
	// C0 - CF
	{zRET,"nz"},       {zPOP,".bc"},      {zBNZ,"A"},          {zBP,"A"},
	{zCALL,"nz,A"},    {zPUSH,".bc"},     {zADD,".a,B"},       {zBRK,"V"},
	{zRET,"z"},        {zRET,nullptr},    {zBZ,"A"},           {zDB,"cb"},
	{zCALL,"z,A"},     {zCALL,"A"},       {zADDC,".a,B"},      {zBRK,"V"},
	// D0 - DF
	{zRET,"nc"},       {zPOP,".de"},      {zBNC,"A"},          {zOUT,"[P],[.a]"},
	{zCALL,"nc,A"},    {zPUSH,".de"},     {zSUB,".a,B"},       {zBRK,"V"},
	{zRET,"c"},        {zXCHX,nullptr},   {zBC,"A"},           {zIN,".a,[P]"},
	{zCALL,"c,A"},     {zDB,"dd"},        {zSUBC,".a,B"},      {zBRK,"V"},
	// E0 - EF
	{zRET,"po"},       {zPOP,".hl"},      {zBPO,"A"},          {zXCH,"[.sp],.hl"},
	{zCALL,"po,A"},    {zPUSH,".hl"},     {zAND,".a,B"},       {zBRK,"V"},
	{zRET,"pe"},       {zBR,"[.hl]"},     {zBPE,"A"},          {zXCH,".de,.hl"},
	{zCALL,"pe,A"},    {zDB,"ed"},        {zXOR,".a,B"},       {zBRK,"V"},
	// F0 - FF
	{zRET,"p"},        {zPOP,".af"},      {zBP,"A"},           {zDI,nullptr},
	{zCALL,"p,A"},     {zPUSH,".af"},     {zOR,".a,B"},        {zBRK,"V"},
	{zRET,"m"},        {zLD,".sp,.hl"},   {zBM,"A"},           {zEI,nullptr},
	{zCALL,"m,A"},     {zDB,"fd"},        {zCMP,".a,B"},       {zBRK,"V"}
};

char r800_disassembler::sign(s8 offset)
{
	return (offset < 0) ? '-' : '+';
}

u32 r800_disassembler::offs(s8 offset)
{
	if (offset < 0)
		return -offset;
	return offset;
}

r800_disassembler::r800_disassembler()
{
}

u32 r800_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t r800_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	s8 offset = 0;

	offs_t pos = pc;
	std::string ixy = "oops!!";

	const r800dasm *d = nullptr;
	u8 op = opcodes.r8(pos++);
	switch (op)
	{
	case 0xcb:
		op = opcodes.r8(pos++);
		d = &mnemonic_cb[op];
		break;
	case 0xed:
		d = &mnemonic_ed[opcodes.r8(pos++)];
		if (d->mnemonic == zDB)
			pos--;
		break;
	case 0xdd:
	case 0xfd:
	{
		ixy = (op == 0xdd) ? "ix" : "iy";
		u8 op1 = opcodes.r8(pos++);
		if (op1 == 0xcb)
		{
			offset = params.r8(pos++);
			op1 = params.r8(pos++);
			d = &mnemonic_xx_cb[op1];
		}
		else
		{
			d = &mnemonic_xx[op1];
			if (d->mnemonic == zDB)
				pos--;
		}
		break;
	}
	default:
		d = &mnemonic_main[op];
		break;
	}

	uint32_t flags = s_flags[d->mnemonic];
	if (d->arguments)
	{
		util::stream_format(stream, "%-5s ", s_mnemonic[d->mnemonic]);
		const char *src = d->arguments;
		while (*src)
		{
			switch (*src)
			{
			case '?': // illegal opcode
				util::stream_format(stream, "$%02x", op);
				break;
			case 'A':
				util::stream_format(stream, "$%04X", params.r16(pos));
				pos += 2;
				if (src != d->arguments)
					flags |= STEP_COND;
				break;
			case 'B': // Byte op arg
				util::stream_format(stream, "$%02X", params.r8(pos++));
				break;
			case 'N': // Immediate 16 bit
				util::stream_format(stream, "$%04X", params.r16(pos));
				pos += 2;
				break;
			case 'O': // Offset relative to PC
				util::stream_format(stream, "$%04X", (pc + s8(params.r8(pos++)) + 2) & 0xffff);
				if (src != d->arguments)
					flags |= STEP_COND;
				break;
			case 'P': // Port number
				util::stream_format(stream, "$%02X", params.r8(pos++));
				break;
			case 'V': // Break vector
				util::stream_format(stream, "$%02X", op & 0x38);
				break;
			case 'W': // Memory address word
				util::stream_format(stream, "$%04X", params.r16(pos));
				pos += 2;
				break;
			case 'X':
				offset = params.r8(pos++);
				[[fallthrough]];
			case 'Y':
				util::stream_format(stream,"[.%s%c$%02x]", ixy, sign(offset), offs(offset));
				break;
			case 'I':
				util::stream_format(stream, "%s", ixy);
				break;
			default:
				stream << *src;
				break;
			}
			src++;
		}
		if (d->mnemonic == zRET)
			flags |= STEP_COND;
	}
	else
	{
		util::stream_format(stream, "%s", s_mnemonic[d->mnemonic]);
	}

	return (pos - pc) | flags | SUPPORTED;
}
