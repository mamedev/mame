// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   z180dasm.c
 *   Portable Z8x180 disassembler
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "z180.h"

enum e_mnemonics {
	zADC   ,zADD   ,zAND   ,zBIT   ,zCALL  ,zCCF   ,zCP    ,zCPD   ,
	zCPDR  ,zCPI   ,zCPIR  ,zCPL   ,zDAA   ,zDB    ,zDEC   ,zDI    ,
	zDJNZ  ,zEI    ,zEX    ,zEXX   ,zHLT   ,zIM    ,zIN    ,zIN0   ,
	zINC   ,zIND   ,zINDR  ,zINI   ,zINIR  ,zJP    ,zJR    ,zLD    ,
	zLDD   ,zLDDR  ,zLDI   ,zLDIR  ,zMLT   ,zNEG   ,zNOP   ,zOR    ,
	zOTDM  ,zOTDMR ,zOTDR  ,zOTIM  ,zOTIMR ,zOTIR  ,zOUT   ,zOUT0  ,
	zOUTD  ,zOUTI  ,zPOP   ,zPUSH  ,zRES   ,zRET   ,zRETI  ,zRETN  ,
	zRL    ,zRLA   ,zRLC   ,zRLCA  ,zRLD   ,zRR    ,zRRA   ,zRRC   ,
	zRRCA  ,zRRD   ,zRST   ,zSBC   ,zSCF   ,zSET   ,zSLA   ,zSLL   ,
	zSLP   ,zSRA   ,zSRL   ,zSUB   ,zTST   ,zTSTIO ,zXOR
};

static const char *const s_mnemonic[] = {
	"adc"  ,"add"  ,"and"  ,"bit"  ,"call" ,"ccf"  ,"cp"   ,"cpd"  ,
	"cpdr" ,"cpi"  ,"cpir" ,"cpl"  ,"daa"  ,"db"   ,"dec"  ,"di"   ,
	"djnz" ,"ei"   ,"ex"   ,"exx"  ,"halt" ,"im"   ,"in"   ,"in0"  ,
	"inc"  ,"ind"  ,"indr" ,"ini"  ,"inir" ,"jp"   ,"jr"   ,"ld"   ,
	"ldd"  ,"lddr" ,"ldi"  ,"ldir" ,"mlt"  ,"neg"  ,"nop"  ,"or"   ,
	"otdm" ,"otdmr","otdr" ,"otim" ,"otimr","otir" ,"out"  ,"out0" ,
	"outd" ,"outi" ,"pop"  ,"push" ,"res"  ,"ret"  ,"reti" ,"retn" ,
	"rl"   ,"rla"  ,"rlc"  ,"rlca" ,"rld"  ,"rr"   ,"rra"  ,"rrc"  ,
	"rrca" ,"rrd"  ,"rst"  ,"sbc"  ,"scf"  ,"set"  ,"sla"  ,"sll"  ,
	"slp"  ,"sra"  ,"srl"  ,"sub"  ,"tst"  ,"tstio","xor "
};

struct z80dasm {
	UINT8 mnemonic;
	const char *arguments;
};

static const z80dasm mnemonic_xx_cb[256]= {
	{zRLC,"b=Y"},   {zRLC,"c=Y"},   {zRLC,"d=Y"},   {zRLC,"e=Y"},
	{zRLC,"h=Y"},   {zRLC,"l=Y"},   {zRLC,"Y"},     {zRLC,"a=Y"},
	{zRRC,"b=Y"},   {zRRC,"c=Y"},   {zRRC,"d=Y"},   {zRRC,"e=Y"},
	{zRRC,"h=Y"},   {zRRC,"l=Y"},   {zRRC,"Y"},     {zRRC,"a=Y"},
	{zRL,"b=Y"},    {zRL,"c=Y"},    {zRL,"d=Y"},    {zRL,"e=Y"},
	{zRL,"h=Y"},    {zRL,"l=Y"},    {zRL,"Y"},      {zRL,"a=Y"},
	{zRR,"b=Y"},    {zRR,"c=Y"},    {zRR,"d=Y"},    {zRR,"e=Y"},
	{zRR,"h=Y"},    {zRR,"l=Y"},    {zRR,"Y"},      {zRR,"a=Y"},
	{zSLA,"b=Y"},   {zSLA,"c=Y"},   {zSLA,"d=Y"},   {zSLA,"e=Y"},
	{zSLA,"h=Y"},   {zSLA,"l=Y"},   {zSLA,"Y"},     {zSLA,"a=Y"},
	{zSRA,"b=Y"},   {zSRA,"c=Y"},   {zSRA,"d=Y"},   {zSRA,"e=Y"},
	{zSRA,"h=Y"},   {zSRA,"l=Y"},   {zSRA,"Y"},     {zSRA,"a=Y"},
	{zSLL,"b=Y"},   {zSLL,"c=Y"},   {zSLL,"d=Y"},   {zSLL,"e=Y"},
	{zSLL,"h=Y"},   {zSLL,"l=Y"},   {zSLL,"Y"},     {zSLL,"a=Y"},
	{zSRL,"b=Y"},   {zSRL,"c=Y"},   {zSRL,"d=Y"},   {zSRL,"e=Y"},
	{zSRL,"h=Y"},   {zSRL,"l=Y"},   {zSRL,"Y"},     {zSRL,"a=Y"},
	{zBIT,"b=0,Y"}, {zBIT,"c=0,Y"}, {zBIT,"d=0,Y"}, {zBIT,"e=0,Y"},
	{zBIT,"h=0,Y"}, {zBIT,"l=0,Y"}, {zBIT,"0,Y"},   {zBIT,"a=0,Y"},
	{zBIT,"b=1,Y"}, {zBIT,"c=1,Y"}, {zBIT,"d=1,Y"}, {zBIT,"e=1,Y"},
	{zBIT,"h=1,Y"}, {zBIT,"l=1,Y"}, {zBIT,"1,Y"},   {zBIT,"a=1,Y"},
	{zBIT,"b=2,Y"}, {zBIT,"c=2,Y"}, {zBIT,"d=2,Y"}, {zBIT,"e=2,Y"},
	{zBIT,"h=2,Y"}, {zBIT,"l=2,Y"}, {zBIT,"2,Y"},   {zBIT,"a=2,Y"},
	{zBIT,"b=3,Y"}, {zBIT,"c=3,Y"}, {zBIT,"d=3,Y"}, {zBIT,"e=3,Y"},
	{zBIT,"h=3,Y"}, {zBIT,"l=3,Y"}, {zBIT,"3,Y"},   {zBIT,"a=3,Y"},
	{zBIT,"b=4,Y"}, {zBIT,"c=4,Y"}, {zBIT,"d=4,Y"}, {zBIT,"e=4,Y"},
	{zBIT,"h=4,Y"}, {zBIT,"l=4,Y"}, {zBIT,"4,Y"},   {zBIT,"a=4,Y"},
	{zBIT,"b=5,Y"}, {zBIT,"c=5,Y"}, {zBIT,"d=5,Y"}, {zBIT,"e=5,Y"},
	{zBIT,"h=5,Y"}, {zBIT,"l=5,Y"}, {zBIT,"5,Y"},   {zBIT,"a=5,Y"},
	{zBIT,"b=6,Y"}, {zBIT,"c=6,Y"}, {zBIT,"d=6,Y"}, {zBIT,"e=6,Y"},
	{zBIT,"h=6,Y"}, {zBIT,"l=6,Y"}, {zBIT,"6,Y"},   {zBIT,"a=6,Y"},
	{zBIT,"b=7,Y"}, {zBIT,"c=7,Y"}, {zBIT,"d=7,Y"}, {zBIT,"e=7,Y"},
	{zBIT,"h=7,Y"}, {zBIT,"l=7,Y"}, {zBIT,"7,Y"},   {zBIT,"a=7,Y"},
	{zRES,"b=0,Y"}, {zRES,"c=0,Y"}, {zRES,"d=0,Y"}, {zRES,"e=0,Y"},
	{zRES,"h=0,Y"}, {zRES,"l=0,Y"}, {zRES,"0,Y"},   {zRES,"a=0,Y"},
	{zRES,"b=1,Y"}, {zRES,"c=1,Y"}, {zRES,"d=1,Y"}, {zRES,"e=1,Y"},
	{zRES,"h=1,Y"}, {zRES,"l=1,Y"}, {zRES,"1,Y"},   {zRES,"a=1,Y"},
	{zRES,"b=2,Y"}, {zRES,"c=2,Y"}, {zRES,"d=2,Y"}, {zRES,"e=2,Y"},
	{zRES,"h=2,Y"}, {zRES,"l=2,Y"}, {zRES,"2,Y"},   {zRES,"a=2,Y"},
	{zRES,"b=3,Y"}, {zRES,"c=3,Y"}, {zRES,"d=3,Y"}, {zRES,"e=3,Y"},
	{zRES,"h=3,Y"}, {zRES,"l=3,Y"}, {zRES,"3,Y"},   {zRES,"a=3,Y"},
	{zRES,"b=4,Y"}, {zRES,"c=4,Y"}, {zRES,"d=4,Y"}, {zRES,"e=4,Y"},
	{zRES,"h=4,Y"}, {zRES,"l=4,Y"}, {zRES,"4,Y"},   {zRES,"a=4,Y"},
	{zRES,"b=5,Y"}, {zRES,"c=5,Y"}, {zRES,"d=5,Y"}, {zRES,"e=5,Y"},
	{zRES,"h=5,Y"}, {zRES,"l=5,Y"}, {zRES,"5,Y"},   {zRES,"a=5,Y"},
	{zRES,"b=6,Y"}, {zRES,"c=6,Y"}, {zRES,"d=6,Y"}, {zRES,"e=6,Y"},
	{zRES,"h=6,Y"}, {zRES,"l=6,Y"}, {zRES,"6,Y"},   {zRES,"a=6,Y"},
	{zRES,"b=7,Y"}, {zRES,"c=7,Y"}, {zRES,"d=7,Y"}, {zRES,"e=7,Y"},
	{zRES,"h=7,Y"}, {zRES,"l=7,Y"}, {zRES,"7,Y"},   {zRES,"a=7,Y"},
	{zSET,"b=0,Y"}, {zSET,"c=0,Y"}, {zSET,"d=0,Y"}, {zSET,"e=0,Y"},
	{zSET,"h=0,Y"}, {zSET,"l=0,Y"}, {zSET,"0,Y"},   {zSET,"a=0,Y"},
	{zSET,"b=1,Y"}, {zSET,"c=1,Y"}, {zSET,"d=1,Y"}, {zSET,"e=1,Y"},
	{zSET,"h=1,Y"}, {zSET,"l=1,Y"}, {zSET,"1,Y"},   {zSET,"a=1,Y"},
	{zSET,"b=2,Y"}, {zSET,"c=2,Y"}, {zSET,"d=2,Y"}, {zSET,"e=2,Y"},
	{zSET,"h=2,Y"}, {zSET,"l=2,Y"}, {zSET,"2,Y"},   {zSET,"a=2,Y"},
	{zSET,"b=3,Y"}, {zSET,"c=3,Y"}, {zSET,"d=3,Y"}, {zSET,"e=3,Y"},
	{zSET,"h=3,Y"}, {zSET,"l=3,Y"}, {zSET,"3,Y"},   {zSET,"a=3,Y"},
	{zSET,"b=4,Y"}, {zSET,"c=4,Y"}, {zSET,"d=4,Y"}, {zSET,"e=4,Y"},
	{zSET,"h=4,Y"}, {zSET,"l=4,Y"}, {zSET,"4,Y"},   {zSET,"a=4,Y"},
	{zSET,"b=5,Y"}, {zSET,"c=5,Y"}, {zSET,"d=5,Y"}, {zSET,"e=5,Y"},
	{zSET,"h=5,Y"}, {zSET,"l=5,Y"}, {zSET,"5,Y"},   {zSET,"a=5,Y"},
	{zSET,"b=6,Y"}, {zSET,"c=6,Y"}, {zSET,"d=6,Y"}, {zSET,"e=6,Y"},
	{zSET,"h=6,Y"}, {zSET,"l=6,Y"}, {zSET,"6,Y"},   {zSET,"a=6,Y"},
	{zSET,"b=7,Y"}, {zSET,"c=7,Y"}, {zSET,"d=7,Y"}, {zSET,"e=7,Y"},
	{zSET,"h=7,Y"}, {zSET,"l=7,Y"}, {zSET,"7,Y"},   {zSET,"a=7,Y"}
};

static const z80dasm mnemonic_cb[256] = {
	{zRLC,"b"},     {zRLC,"c"},     {zRLC,"d"},     {zRLC,"e"},
	{zRLC,"h"},     {zRLC,"l"},     {zRLC,"(hl)"},  {zRLC,"a"},
	{zRRC,"b"},     {zRRC,"c"},     {zRRC,"d"},     {zRRC,"e"},
	{zRRC,"h"},     {zRRC,"l"},     {zRRC,"(hl)"},  {zRRC,"a"},
	{zRL,"b"},      {zRL,"c"},      {zRL,"d"},      {zRL,"e"},
	{zRL,"h"},      {zRL,"l"},      {zRL,"(hl)"},   {zRL,"a"},
	{zRR,"b"},      {zRR,"c"},      {zRR,"d"},      {zRR,"e"},
	{zRR,"h"},      {zRR,"l"},      {zRR,"(hl)"},   {zRR,"a"},
	{zSLA,"b"},     {zSLA,"c"},     {zSLA,"d"},     {zSLA,"e"},
	{zSLA,"h"},     {zSLA,"l"},     {zSLA,"(hl)"},  {zSLA,"a"},
	{zSRA,"b"},     {zSRA,"c"},     {zSRA,"d"},     {zSRA,"e"},
	{zSRA,"h"},     {zSRA,"l"},     {zSRA,"(hl)"},  {zSRA,"a"},
	{zSLL,"b"},     {zSLL,"c"},     {zSLL,"d"},     {zSLL,"e"},
	{zSLL,"h"},     {zSLL,"l"},     {zSLL,"(hl)"},  {zSLL,"a"},
	{zSRL,"b"},     {zSRL,"c"},     {zSRL,"d"},     {zSRL,"e"},
	{zSRL,"h"},     {zSRL,"l"},     {zSRL,"(hl)"},  {zSRL,"a"},
	{zBIT,"0,b"},   {zBIT,"0,c"},   {zBIT,"0,d"},   {zBIT,"0,e"},
	{zBIT,"0,h"},   {zBIT,"0,l"},   {zBIT,"0,(hl)"},{zBIT,"0,a"},
	{zBIT,"1,b"},   {zBIT,"1,c"},   {zBIT,"1,d"},   {zBIT,"1,e"},
	{zBIT,"1,h"},   {zBIT,"1,l"},   {zBIT,"1,(hl)"},{zBIT,"1,a"},
	{zBIT,"2,b"},   {zBIT,"2,c"},   {zBIT,"2,d"},   {zBIT,"2,e"},
	{zBIT,"2,h"},   {zBIT,"2,l"},   {zBIT,"2,(hl)"},{zBIT,"2,a"},
	{zBIT,"3,b"},   {zBIT,"3,c"},   {zBIT,"3,d"},   {zBIT,"3,e"},
	{zBIT,"3,h"},   {zBIT,"3,l"},   {zBIT,"3,(hl)"},{zBIT,"3,a"},
	{zBIT,"4,b"},   {zBIT,"4,c"},   {zBIT,"4,d"},   {zBIT,"4,e"},
	{zBIT,"4,h"},   {zBIT,"4,l"},   {zBIT,"4,(hl)"},{zBIT,"4,a"},
	{zBIT,"5,b"},   {zBIT,"5,c"},   {zBIT,"5,d"},   {zBIT,"5,e"},
	{zBIT,"5,h"},   {zBIT,"5,l"},   {zBIT,"5,(hl)"},{zBIT,"5,a"},
	{zBIT,"6,b"},   {zBIT,"6,c"},   {zBIT,"6,d"},   {zBIT,"6,e"},
	{zBIT,"6,h"},   {zBIT,"6,l"},   {zBIT,"6,(hl)"},{zBIT,"6,a"},
	{zBIT,"7,b"},   {zBIT,"7,c"},   {zBIT,"7,d"},   {zBIT,"7,e"},
	{zBIT,"7,h"},   {zBIT,"7,l"},   {zBIT,"7,(hl)"},{zBIT,"7,a"},
	{zRES,"0,b"},   {zRES,"0,c"},   {zRES,"0,d"},   {zRES,"0,e"},
	{zRES,"0,h"},   {zRES,"0,l"},   {zRES,"0,(hl)"},{zRES,"0,a"},
	{zRES,"1,b"},   {zRES,"1,c"},   {zRES,"1,d"},   {zRES,"1,e"},
	{zRES,"1,h"},   {zRES,"1,l"},   {zRES,"1,(hl)"},{zRES,"1,a"},
	{zRES,"2,b"},   {zRES,"2,c"},   {zRES,"2,d"},   {zRES,"2,e"},
	{zRES,"2,h"},   {zRES,"2,l"},   {zRES,"2,(hl)"},{zRES,"2,a"},
	{zRES,"3,b"},   {zRES,"3,c"},   {zRES,"3,d"},   {zRES,"3,e"},
	{zRES,"3,h"},   {zRES,"3,l"},   {zRES,"3,(hl)"},{zRES,"3,a"},
	{zRES,"4,b"},   {zRES,"4,c"},   {zRES,"4,d"},   {zRES,"4,e"},
	{zRES,"4,h"},   {zRES,"4,l"},   {zRES,"4,(hl)"},{zRES,"4,a"},
	{zRES,"5,b"},   {zRES,"5,c"},   {zRES,"5,d"},   {zRES,"5,e"},
	{zRES,"5,h"},   {zRES,"5,l"},   {zRES,"5,(hl)"},{zRES,"5,a"},
	{zRES,"6,b"},   {zRES,"6,c"},   {zRES,"6,d"},   {zRES,"6,e"},
	{zRES,"6,h"},   {zRES,"6,l"},   {zRES,"6,(hl)"},{zRES,"6,a"},
	{zRES,"7,b"},   {zRES,"7,c"},   {zRES,"7,d"},   {zRES,"7,e"},
	{zRES,"7,h"},   {zRES,"7,l"},   {zRES,"7,(hl)"},{zRES,"7,a"},
	{zSET,"0,b"},   {zSET,"0,c"},   {zSET,"0,d"},   {zSET,"0,e"},
	{zSET,"0,h"},   {zSET,"0,l"},   {zSET,"0,(hl)"},{zSET,"0,a"},
	{zSET,"1,b"},   {zSET,"1,c"},   {zSET,"1,d"},   {zSET,"1,e"},
	{zSET,"1,h"},   {zSET,"1,l"},   {zSET,"1,(hl)"},{zSET,"1,a"},
	{zSET,"2,b"},   {zSET,"2,c"},   {zSET,"2,d"},   {zSET,"2,e"},
	{zSET,"2,h"},   {zSET,"2,l"},   {zSET,"2,(hl)"},{zSET,"2,a"},
	{zSET,"3,b"},   {zSET,"3,c"},   {zSET,"3,d"},   {zSET,"3,e"},
	{zSET,"3,h"},   {zSET,"3,l"},   {zSET,"3,(hl)"},{zSET,"3,a"},
	{zSET,"4,b"},   {zSET,"4,c"},   {zSET,"4,d"},   {zSET,"4,e"},
	{zSET,"4,h"},   {zSET,"4,l"},   {zSET,"4,(hl)"},{zSET,"4,a"},
	{zSET,"5,b"},   {zSET,"5,c"},   {zSET,"5,d"},   {zSET,"5,e"},
	{zSET,"5,h"},   {zSET,"5,l"},   {zSET,"5,(hl)"},{zSET,"5,a"},
	{zSET,"6,b"},   {zSET,"6,c"},   {zSET,"6,d"},   {zSET,"6,e"},
	{zSET,"6,h"},   {zSET,"6,l"},   {zSET,"6,(hl)"},{zSET,"6,a"},
	{zSET,"7,b"},   {zSET,"7,c"},   {zSET,"7,d"},   {zSET,"7,e"},
	{zSET,"7,h"},   {zSET,"7,l"},   {zSET,"7,(hl)"},{zSET,"7,a"}
};

static const z80dasm mnemonic_ed[256]= {
	{zIN0,"b,(B)"}, {zOUT0,"(B),b"},{zDB,"?"},      {zDB,"?"},
	{zTST,"b"},     {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zIN0,"c,(B)"}, {zOUT0,"(B),c"},{zDB,"?"},      {zDB,"?"},
	{zTST,"c"},     {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zIN0,"d,(B)"}, {zOUT0,"(B),d"},{zDB,"?"},      {zDB,"?"},
	{zTST,"d"},     {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zIN0,"e,(B)"}, {zOUT0,"(B),e"},{zDB,"?"},      {zDB,"?"},
	{zTST,"e"},     {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zIN0,"h,(B)"}, {zOUT0,"(B),h"},{zDB,"?"},      {zDB,"?"},
	{zTST,"h"},     {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zIN0,"l,(B)"}, {zOUT0,"(B),l"},{zDB,"?"},      {zDB,"?"},
	{zTST,"l"},     {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zIN0,"f,(B)"}, {zOUT0,"(B),0"},{zDB,"?"},      {zDB,"?"},
	{zTST,"(hl)"},  {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zIN0,"a,(B)"}, {zOUT0,"(B),a"},{zDB,"?"},      {zDB,"?"},
	{zTST,"a"},     {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zIN,"b,(c)"},  {zOUT,"(c),b"}, {zSBC,"hl,bc"}, {zLD,"(W),bc"},
	{zNEG,nullptr},       {zRETN,nullptr},      {zIM,"0"},      {zLD,"i,a"},
	{zIN,"c,(c)"},  {zOUT,"(c),c"}, {zADC,"hl,bc"}, {zLD,"bc,(W)"},
	{zMLT,"bc"},    {zRETI,nullptr},      {zIM,"0"},      {zLD,"r,a"},
	{zIN,"d,(c)"},  {zOUT,"(c),d"}, {zSBC,"hl,de"}, {zLD,"(W),de"},
	{zNEG,"*"},     {zRETN,nullptr},      {zIM,"1"},      {zLD,"a,i"},
	{zIN,"e,(c)"},  {zOUT,"(c),e"}, {zADC,"hl,de"}, {zLD,"de,(W)"},
	{zMLT,"de"},    {zRETI,nullptr},      {zIM,"2"},      {zLD,"a,r"},
	{zIN,"h,(c)"},  {zOUT,"(c),h"}, {zSBC,"hl,hl"}, {zLD,"(W),hl"},
	{zTST,"B"},     {zRETN,nullptr},      {zIM,"0"},      {zRRD,"(hl)"},
	{zIN,"l,(c)"},  {zOUT,"(c),l"}, {zADC,"hl,hl"}, {zLD,"hl,(W)"},
	{zMLT,"hl"},    {zRETI,nullptr},      {zIM,"0"},      {zRLD,"(hl)"},
	{zIN,"0,(c)"},  {zOUT,"(c),0"}, {zSBC,"hl,sp"}, {zLD,"(W),sp"},
	{zTSTIO,"B"},   {zRETN,nullptr},      {zSLP,nullptr},       {zDB,"?"},
	{zIN,"a,(c)"},  {zOUT,"(c),a"}, {zADC,"hl,sp"}, {zLD,"sp,(W)"},
	{zMLT,"sp"},    {zRETI,nullptr},      {zIM,"2"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zOTIM,nullptr},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zOTDM,nullptr},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zOTIMR,nullptr},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zOTDMR,nullptr},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zLDI,nullptr},       {zCPI,nullptr},       {zINI,nullptr},       {zOUTI,nullptr},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zLDD,nullptr},       {zCPD,nullptr},       {zIND,nullptr},       {zOUTD,nullptr},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zLDIR,nullptr},      {zCPIR,nullptr},      {zINIR,nullptr},      {zOTIR,nullptr},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zLDDR,nullptr},      {zCPDR,nullptr},      {zINDR,nullptr},      {zOTDR,nullptr},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"}
};

static const z80dasm mnemonic_xx[256]= {
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zADD,"I,bc"},  {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zADD,"I,de"},  {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zLD,"I,N"},    {zLD,"(W),I"},  {zINC,"I"},
	{zINC,"Ih"},    {zDEC,"Ih"},    {zLD,"Ih,B"},   {zDB,"?"},
	{zDB,"?"},      {zADD,"I,I"},   {zLD,"I,(W)"},  {zDEC,"I"},
	{zINC,"Il"},    {zDEC,"Il"},    {zLD,"Il,B"},   {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zINC,"X"},     {zDEC,"X"},     {zLD,"X,B"},    {zDB,"?"},
	{zDB,"?"},      {zADD,"I,sp"},  {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zLD,"b,Ih"},   {zLD,"b,Il"},   {zLD,"b,X"},    {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zLD,"c,Ih"},   {zLD,"c,Il"},   {zLD,"c,X"},    {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zLD,"d,Ih"},   {zLD,"d,Il"},   {zLD,"d,X"},    {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zLD,"e,Ih"},   {zLD,"e,Il"},   {zLD,"e,X"},    {zDB,"?"},
	{zLD,"Ih,b"},   {zLD,"Ih,c"},   {zLD,"Ih,d"},   {zLD,"Ih,e"},
	{zLD,"Ih,Ih"},  {zLD,"Ih,Il"},  {zLD,"h,X"},    {zLD,"Ih,a"},
	{zLD,"Il,b"},   {zLD,"Il,c"},   {zLD,"Il,d"},   {zLD,"Il,e"},
	{zLD,"Il,Ih"},  {zLD,"Il,Il"},  {zLD,"l,X"},    {zLD,"Il,a"},
	{zLD,"X,b"},    {zLD,"X,c"},    {zLD,"X,d"},    {zLD,"X,e"},
	{zLD,"X,h"},    {zLD,"X,l"},    {zDB,"?"},      {zLD,"X,a"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zLD,"a,Ih"},   {zLD,"a,Il"},   {zLD,"a,X"},    {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zADD,"a,Ih"},  {zADD,"a,Il"},  {zADD,"a,X"},   {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zADC,"a,Ih"},  {zADC,"a,Il"},  {zADC,"a,X"},   {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zSUB,"Ih"},    {zSUB,"Il"},    {zSUB,"X"},     {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zSBC,"a,Ih"},  {zSBC,"a,Il"},  {zSBC,"a,X"},   {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zAND,"Ih"},    {zAND,"Il"},    {zAND,"X"},     {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zXOR,"Ih"},    {zXOR,"Il"},    {zXOR,"X"},     {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zOR,"Ih"},     {zOR,"Il"},     {zOR,"X"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zCP,"Ih"},     {zCP,"Il"},     {zCP,"X"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"cb"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zPOP,"I"},     {zDB,"?"},      {zEX,"(sp),I"},
	{zDB,"?"},      {zPUSH,"I"},    {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zJP,"(I)"},    {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zLD,"sp,I"},   {zDB,"?"},      {zDB,"?"},
	{zDB,"?"},      {zDB,"?"},      {zDB,"?"},      {zDB,"?"}
};

static const z80dasm mnemonic_main[256]= {
	{zNOP,nullptr},       {zLD,"bc,N"},   {zLD,"(bc),a"}, {zINC,"bc"},
	{zINC,"b"},     {zDEC,"b"},     {zLD,"b,B"},    {zRLCA,nullptr},
	{zEX,"af,af'"}, {zADD,"hl,bc"}, {zLD,"a,(bc)"}, {zDEC,"bc"},
	{zINC,"c"},     {zDEC,"c"},     {zLD,"c,B"},    {zRRCA,nullptr},
	{zDJNZ,"O"},    {zLD,"de,N"},   {zLD,"(de),a"}, {zINC,"de"},
	{zINC,"d"},     {zDEC,"d"},     {zLD,"d,B"},    {zRLA,nullptr},
	{zJR,"O"},      {zADD,"hl,de"}, {zLD,"a,(de)"}, {zDEC,"de"},
	{zINC,"e"},     {zDEC,"e"},     {zLD,"e,B"},    {zRRA,nullptr},
	{zJR,"nz,O"},   {zLD,"hl,N"},   {zLD,"(W),hl"}, {zINC,"hl"},
	{zINC,"h"},     {zDEC,"h"},     {zLD,"h,B"},    {zDAA,nullptr},
	{zJR,"z,O"},    {zADD,"hl,hl"}, {zLD,"hl,(W)"}, {zDEC,"hl"},
	{zINC,"l"},     {zDEC,"l"},     {zLD,"l,B"},    {zCPL,nullptr},
	{zJR,"nc,O"},   {zLD,"sp,N"},   {zLD,"(W),a"},  {zINC,"sp"},
	{zINC,"(hl)"},  {zDEC,"(hl)"},  {zLD,"(hl),B"}, {zSCF,nullptr},
	{zJR,"c,O"},    {zADD,"hl,sp"}, {zLD,"a,(W)"},  {zDEC,"sp"},
	{zINC,"a"},     {zDEC,"a"},     {zLD,"a,B"},    {zCCF,nullptr},
	{zLD,"b,b"},    {zLD,"b,c"},    {zLD,"b,d"},    {zLD,"b,e"},
	{zLD,"b,h"},    {zLD,"b,l"},    {zLD,"b,(hl)"}, {zLD,"b,a"},
	{zLD,"c,b"},    {zLD,"c,c"},    {zLD,"c,d"},    {zLD,"c,e"},
	{zLD,"c,h"},    {zLD,"c,l"},    {zLD,"c,(hl)"}, {zLD,"c,a"},
	{zLD,"d,b"},    {zLD,"d,c"},    {zLD,"d,d"},    {zLD,"d,e"},
	{zLD,"d,h"},    {zLD,"d,l"},    {zLD,"d,(hl)"}, {zLD,"d,a"},
	{zLD,"e,b"},    {zLD,"e,c"},    {zLD,"e,d"},    {zLD,"e,e"},
	{zLD,"e,h"},    {zLD,"e,l"},    {zLD,"e,(hl)"}, {zLD,"e,a"},
	{zLD,"h,b"},    {zLD,"h,c"},    {zLD,"h,d"},    {zLD,"h,e"},
	{zLD,"h,h"},    {zLD,"h,l"},    {zLD,"h,(hl)"}, {zLD,"h,a"},
	{zLD,"l,b"},    {zLD,"l,c"},    {zLD,"l,d"},    {zLD,"l,e"},
	{zLD,"l,h"},    {zLD,"l,l"},    {zLD,"l,(hl)"}, {zLD,"l,a"},
	{zLD,"(hl),b"}, {zLD,"(hl),c"}, {zLD,"(hl),d"}, {zLD,"(hl),e"},
	{zLD,"(hl),h"}, {zLD,"(hl),l"}, {zHLT,nullptr},       {zLD,"(hl),a"},
	{zLD,"a,b"},    {zLD,"a,c"},    {zLD,"a,d"},    {zLD,"a,e"},
	{zLD,"a,h"},    {zLD,"a,l"},    {zLD,"a,(hl)"}, {zLD,"a,a"},
	{zADD,"a,b"},   {zADD,"a,c"},   {zADD,"a,d"},   {zADD,"a,e"},
	{zADD,"a,h"},   {zADD,"a,l"},   {zADD,"a,(hl)"},{zADD,"a,a"},
	{zADC,"a,b"},   {zADC,"a,c"},   {zADC,"a,d"},   {zADC,"a,e"},
	{zADC,"a,h"},   {zADC,"a,l"},   {zADC,"a,(hl)"},{zADC,"a,a"},
	{zSUB,"b"},     {zSUB,"c"},     {zSUB,"d"},     {zSUB,"e"},
	{zSUB,"h"},     {zSUB,"l"},     {zSUB,"(hl)"},  {zSUB,"a"},
	{zSBC,"a,b"},   {zSBC,"a,c"},   {zSBC,"a,d"},   {zSBC,"a,e"},
	{zSBC,"a,h"},   {zSBC,"a,l"},   {zSBC,"a,(hl)"},{zSBC,"a,a"},
	{zAND,"b"},     {zAND,"c"},     {zAND,"d"},     {zAND,"e"},
	{zAND,"h"},     {zAND,"l"},     {zAND,"(hl)"},  {zAND,"a"},
	{zXOR,"b"},     {zXOR,"c"},     {zXOR,"d"},     {zXOR,"e"},
	{zXOR,"h"},     {zXOR,"l"},     {zXOR,"(hl)"},  {zXOR,"a"},
	{zOR,"b"},      {zOR,"c"},      {zOR,"d"},      {zOR,"e"},
	{zOR,"h"},      {zOR,"l"},      {zOR,"(hl)"},   {zOR,"a"},
	{zCP,"b"},      {zCP,"c"},      {zCP,"d"},      {zCP,"e"},
	{zCP,"h"},      {zCP,"l"},      {zCP,"(hl)"},   {zCP,"a"},
	{zRET,"nz"},    {zPOP,"bc"},    {zJP,"nz,A"},   {zJP,"A"},
	{zCALL,"nz,A"}, {zPUSH,"bc"},   {zADD,"a,B"},   {zRST,"V"},
	{zRET,"z"},     {zRET,nullptr},       {zJP,"z,A"},    {zDB,"cb"},
	{zCALL,"z,A"},  {zCALL,"A"},    {zADC,"a,B"},   {zRST,"V"},
	{zRET,"nc"},    {zPOP,"de"},    {zJP,"nc,A"},   {zOUT,"(P),a"},
	{zCALL,"nc,A"}, {zPUSH,"de"},   {zSUB,"B"},     {zRST,"V"},
	{zRET,"c"},     {zEXX,nullptr},       {zJP,"c,A"},    {zIN,"a,(P)"},
	{zCALL,"c,A"},  {zDB,"dd"},     {zSBC,"a,B"},   {zRST,"V"},
	{zRET,"po"},    {zPOP,"hl"},    {zJP,"po,A"},   {zEX,"(sp),hl"},
	{zCALL,"po,A"}, {zPUSH,"hl"},   {zAND,"B"},     {zRST,"V"},
	{zRET,"pe"},    {zJP,"(hl)"},   {zJP,"pe,A"},   {zEX,"de,hl"},
	{zCALL,"pe,A"}, {zDB,"ed"},     {zXOR,"B"},     {zRST,"V"},
	{zRET,"p"},     {zPOP,"af"},    {zJP,"p,A"},    {zDI,nullptr},
	{zCALL,"p,A"},  {zPUSH,"af"},   {zOR,"B"},      {zRST,"V"},
	{zRET,"m"},     {zLD,"sp,hl"},  {zJP,"m,A"},    {zEI,nullptr},
	{zCALL,"m,A"},  {zDB,"fd"},     {zCP,"B"},      {zRST,"V"}
};

static char sign(INT8 offset)
{
	return (offset < 0)? '-':'+';
}

static int offs(INT8 offset)
{
	if (offset < 0) return -offset;
	return offset;
}

/****************************************************************************
 * Disassemble opcode at PC and return number of bytes it takes
 ****************************************************************************/
CPU_DISASSEMBLE( z180 )
{
	const z80dasm *d;
	const char *src, *ixy;
	char *dst;
	unsigned PC = pc;
	INT8 offset = 0;
	UINT8 op, op1 = 0;
	UINT16 ea = 0;
	int pos = 0;
	UINT32 flags = 0;

	ixy = "oops!!";
	dst = buffer;

	op = oprom[pos++];

	switch (op)
	{
	case 0xcb:
		op = oprom[pos++];
		d = &mnemonic_cb[op];
		break;
	case 0xed:
		op1 = oprom[pos++];
		d = &mnemonic_ed[op1];
		break;
	case 0xdd:
		ixy = "ix";
		op1 = oprom[pos++];
		if( op1 == 0xcb )
		{
			offset = (INT8) opram[pos++];
			op1 = opram[pos++]; /* fourth byte from opbase.ram! */
			d = &mnemonic_xx_cb[op1];
		}
		else d = &mnemonic_xx[op1];
		break;
	case 0xfd:
		ixy = "iy";
		op1 = oprom[pos++];
		if( op1 == 0xcb )
		{
			offset = (INT8) opram[pos++];
			op1 = opram[pos++]; /* fourth byte from opbase.ram! */
			d = &mnemonic_xx_cb[op1];
		}
		else d = &mnemonic_xx[op1];
		break;
	default:
		d = &mnemonic_main[op];
		break;
	}

	if( d->arguments )
	{
		dst += sprintf(dst, "%-5s ", s_mnemonic[d->mnemonic]);
		src = d->arguments;
		while( *src )
		{
			switch( *src )
			{
			case '?':   /* illegal opcode */
				dst += sprintf( dst, "$%02x,$%02x", op, op1);
				break;
			case 'A':
				ea = opram[pos] + ( opram[pos+1] << 8);
				pos += 2;
				dst += sprintf( dst, "$%04X", ea );
				break;
			case 'B':   /* Byte op arg */
				ea = opram[pos++];
				dst += sprintf( dst, "$%02X", ea );
				break;
			case 'N':   /* Immediate 16 bit */
				ea = opram[pos] + ( opram[pos+1] << 8 );
				pos += 2;
				dst += sprintf( dst, "$%04X", ea );
				break;
			case 'O':   /* Offset relative to PC */
				offset = (INT8) opram[pos++];
				dst += sprintf( dst, "$%05X", PC + offset + 2 );
				break;
			case 'P':   /* Port number */
				ea = opram[pos++];
				dst += sprintf( dst, "$%02X", ea );
				break;
			case 'V':   /* Restart vector */
				ea = op & 0x38;
				dst += sprintf( dst, "$%02X", ea );
				break;
			case 'W':   /* Memory address word */
				ea = opram[pos] + ( opram[pos+1] << 8);
				pos += 2;
				dst += sprintf( dst, "$%05X", ea );
				break;
			case 'X':
				offset = (INT8) opram[pos++];
			case 'Y':
				dst += sprintf( dst,"(%s%c$%02x)", ixy, sign(offset), offs(offset) );
				break;
			case 'I':
				dst += sprintf( dst, "%s", ixy);
				break;
			default:
				*dst++ = *src;
			}
			src++;
		}
		*dst = '\0';
	}
	else
	{
		dst += sprintf(dst, "%s", s_mnemonic[d->mnemonic]);
	}

	if (d->mnemonic == zCALL || d->mnemonic == zCPDR || d->mnemonic == zCPIR || d->mnemonic == zDJNZ ||
		d->mnemonic == zHLT  || d->mnemonic == zINDR || d->mnemonic == zINIR || d->mnemonic == zLDDR ||
		d->mnemonic == zLDIR || d->mnemonic == zOTDR || d->mnemonic == zOTIR || d->mnemonic == zRST)
		flags = DASMFLAG_STEP_OVER;
	else if (d->mnemonic == zRETN || d->mnemonic == zRET || d->mnemonic == zRETI)
		flags = DASMFLAG_STEP_OUT;

	return pos | flags | DASMFLAG_SUPPORTED;
}
