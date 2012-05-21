static unsigned EA_000(i8086_state *cpustate) { cpustate->icount-=7; cpustate->eo=(WORD)(cpustate->regs.w[BX]+cpustate->regs.w[SI]); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_001(i8086_state *cpustate) { cpustate->icount-=8; cpustate->eo=(WORD)(cpustate->regs.w[BX]+cpustate->regs.w[DI]); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_002(i8086_state *cpustate) { cpustate->icount-=8; cpustate->eo=(WORD)(cpustate->regs.w[BP]+cpustate->regs.w[SI]); cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_003(i8086_state *cpustate) { cpustate->icount-=7; cpustate->eo=(WORD)(cpustate->regs.w[BP]+cpustate->regs.w[DI]); cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_004(i8086_state *cpustate) { cpustate->icount-=5; cpustate->eo=cpustate->regs.w[SI]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_005(i8086_state *cpustate) { cpustate->icount-=5; cpustate->eo=cpustate->regs.w[DI]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_006(i8086_state *cpustate) { cpustate->icount-=6; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_007(i8086_state *cpustate) { cpustate->icount-=5; cpustate->eo=cpustate->regs.w[BX]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }

static unsigned EA_100(i8086_state *cpustate) { cpustate->icount-=11; cpustate->eo=(WORD)(cpustate->regs.w[BX]+cpustate->regs.w[SI]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_101(i8086_state *cpustate) { cpustate->icount-=12; cpustate->eo=(WORD)(cpustate->regs.w[BX]+cpustate->regs.w[DI]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_102(i8086_state *cpustate) { cpustate->icount-=12; cpustate->eo=(WORD)(cpustate->regs.w[BP]+cpustate->regs.w[SI]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_103(i8086_state *cpustate) { cpustate->icount-=11; cpustate->eo=(WORD)(cpustate->regs.w[BP]+cpustate->regs.w[DI]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_104(i8086_state *cpustate) { cpustate->icount-=9; cpustate->eo=(WORD)(cpustate->regs.w[SI]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_105(i8086_state *cpustate) { cpustate->icount-=9; cpustate->eo=(WORD)(cpustate->regs.w[DI]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_106(i8086_state *cpustate) { cpustate->icount-=9; cpustate->eo=(WORD)(cpustate->regs.w[BP]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_107(i8086_state *cpustate) { cpustate->icount-=9; cpustate->eo=(WORD)(cpustate->regs.w[BX]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }

static unsigned EA_200(i8086_state *cpustate) { cpustate->icount-=11; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[BX]+cpustate->regs.w[SI]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+(WORD)cpustate->eo; return cpustate->ea; }
static unsigned EA_201(i8086_state *cpustate) { cpustate->icount-=12; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[BX]+cpustate->regs.w[DI]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+(WORD)cpustate->eo; return cpustate->ea; }
static unsigned EA_202(i8086_state *cpustate) { cpustate->icount-=12; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[BP]+cpustate->regs.w[SI]; cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+(WORD)cpustate->eo; return cpustate->ea; }
static unsigned EA_203(i8086_state *cpustate) { cpustate->icount-=11; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[BP]+cpustate->regs.w[DI]; cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+(WORD)cpustate->eo; return cpustate->ea; }
static unsigned EA_204(i8086_state *cpustate) { cpustate->icount-=9; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[SI]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+(WORD)cpustate->eo; return cpustate->ea; }
static unsigned EA_205(i8086_state *cpustate) { cpustate->icount-=9; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[DI]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+(WORD)cpustate->eo; return cpustate->ea; }
static unsigned EA_206(i8086_state *cpustate) { cpustate->icount-=9; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[BP]; cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+(WORD)cpustate->eo; return cpustate->ea; }
static unsigned EA_207(i8086_state *cpustate) { cpustate->icount-=9; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[BX]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+(WORD)cpustate->eo; return cpustate->ea; }

static unsigned (*const GetEA[192])(i8086_state *cpustate)={
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,

	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,

	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207
};
