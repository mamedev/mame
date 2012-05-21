
static unsigned EA_000(v30mz_state *cpustate) { cpustate->eo=cpustate->regs.w[BW]+cpustate->regs.w[IX]; cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_001(v30mz_state *cpustate) { cpustate->eo=cpustate->regs.w[BW]+cpustate->regs.w[IY]; cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_002(v30mz_state *cpustate) { cpustate->eo=cpustate->regs.w[BP]+cpustate->regs.w[IX]; cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_003(v30mz_state *cpustate) { cpustate->eo=cpustate->regs.w[BP]+cpustate->regs.w[IY]; cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_004(v30mz_state *cpustate) { cpustate->eo=cpustate->regs.w[IX]; cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_005(v30mz_state *cpustate) { cpustate->eo=cpustate->regs.w[IY]; cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_006(v30mz_state *cpustate) { cpustate->eo=FETCH; cpustate->eo+=FETCH<<8; cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_007(v30mz_state *cpustate) { cpustate->eo=cpustate->regs.w[BW]; cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }

static unsigned EA_100(v30mz_state *cpustate) { cpustate->eo=(cpustate->regs.w[BW]+cpustate->regs.w[IX]+(INT8)FETCH); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_101(v30mz_state *cpustate) { cpustate->eo=(cpustate->regs.w[BW]+cpustate->regs.w[IY]+(INT8)FETCH); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_102(v30mz_state *cpustate) { cpustate->eo=(cpustate->regs.w[BP]+cpustate->regs.w[IX]+(INT8)FETCH); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_103(v30mz_state *cpustate) { cpustate->eo=(cpustate->regs.w[BP]+cpustate->regs.w[IY]+(INT8)FETCH); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_104(v30mz_state *cpustate) { cpustate->eo=(cpustate->regs.w[IX]+(INT8)FETCH); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_105(v30mz_state *cpustate) { cpustate->eo=(cpustate->regs.w[IY]+(INT8)FETCH); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_106(v30mz_state *cpustate) { cpustate->eo=(cpustate->regs.w[BP]+(INT8)FETCH); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_107(v30mz_state *cpustate) { cpustate->eo=(cpustate->regs.w[BW]+(INT8)FETCH); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }

static unsigned EA_200(v30mz_state *cpustate) { cpustate->e16=FETCH; cpustate->e16+=FETCH<<8; cpustate->eo=cpustate->regs.w[BW]+cpustate->regs.w[IX]+(INT16)cpustate->e16; cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_201(v30mz_state *cpustate) { cpustate->e16=FETCH; cpustate->e16+=FETCH<<8; cpustate->eo=cpustate->regs.w[BW]+cpustate->regs.w[IY]+(INT16)cpustate->e16; cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_202(v30mz_state *cpustate) { cpustate->e16=FETCH; cpustate->e16+=FETCH<<8; cpustate->eo=cpustate->regs.w[BP]+cpustate->regs.w[IX]+(INT16)cpustate->e16; cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_203(v30mz_state *cpustate) { cpustate->e16=FETCH; cpustate->e16+=FETCH<<8; cpustate->eo=cpustate->regs.w[BP]+cpustate->regs.w[IY]+(INT16)cpustate->e16; cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_204(v30mz_state *cpustate) { cpustate->e16=FETCH; cpustate->e16+=FETCH<<8; cpustate->eo=cpustate->regs.w[IX]+(INT16)cpustate->e16; cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_205(v30mz_state *cpustate) { cpustate->e16=FETCH; cpustate->e16+=FETCH<<8; cpustate->eo=cpustate->regs.w[IY]+(INT16)cpustate->e16; cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_206(v30mz_state *cpustate) { cpustate->e16=FETCH; cpustate->e16+=FETCH<<8; cpustate->eo=cpustate->regs.w[BP]+(INT16)cpustate->e16; cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
static unsigned EA_207(v30mz_state *cpustate) { cpustate->e16=FETCH; cpustate->e16+=FETCH<<8; cpustate->eo=cpustate->regs.w[BW]+(INT16)cpustate->e16; cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }

static unsigned (*const GetEA[192])(v30mz_state *cpustate)={
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
