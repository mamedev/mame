static unsigned EA;
static UINT16 EO; /* HJB 12/13/98 effective offset of the address (before segment is added) */

static unsigned EA_000(void) { i8086_ICount-=7; EO=(WORD)(I.regs.w[BX]+I.regs.w[SI]); EA=DefaultBase(DS)+EO; return EA; }
static unsigned EA_001(void) { i8086_ICount-=8; EO=(WORD)(I.regs.w[BX]+I.regs.w[DI]); EA=DefaultBase(DS)+EO; return EA; }
static unsigned EA_002(void) { i8086_ICount-=8; EO=(WORD)(I.regs.w[BP]+I.regs.w[SI]); EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_003(void) { i8086_ICount-=7; EO=(WORD)(I.regs.w[BP]+I.regs.w[DI]); EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_004(void) { i8086_ICount-=5; EO=I.regs.w[SI]; EA=DefaultBase(DS)+EO; return EA; }
static unsigned EA_005(void) { i8086_ICount-=5; EO=I.regs.w[DI]; EA=DefaultBase(DS)+EO; return EA; }
static unsigned EA_006(void) { i8086_ICount-=6; EO=FETCHOP; EO+=FETCHOP<<8; EA=DefaultBase(DS)+EO; return EA; }
static unsigned EA_007(void) { i8086_ICount-=5; EO=I.regs.w[BX]; EA=DefaultBase(DS)+EO; return EA; }

static unsigned EA_100(void) { i8086_ICount-=11; EO=(WORD)(I.regs.w[BX]+I.regs.w[SI]+(INT8)FETCHOP); EA=DefaultBase(DS)+EO; return EA; }
static unsigned EA_101(void) { i8086_ICount-=12; EO=(WORD)(I.regs.w[BX]+I.regs.w[DI]+(INT8)FETCHOP); EA=DefaultBase(DS)+EO; return EA; }
static unsigned EA_102(void) { i8086_ICount-=12; EO=(WORD)(I.regs.w[BP]+I.regs.w[SI]+(INT8)FETCHOP); EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_103(void) { i8086_ICount-=11; EO=(WORD)(I.regs.w[BP]+I.regs.w[DI]+(INT8)FETCHOP); EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_104(void) { i8086_ICount-=9; EO=(WORD)(I.regs.w[SI]+(INT8)FETCHOP); EA=DefaultBase(DS)+EO; return EA; }
static unsigned EA_105(void) { i8086_ICount-=9; EO=(WORD)(I.regs.w[DI]+(INT8)FETCHOP); EA=DefaultBase(DS)+EO; return EA; }
static unsigned EA_106(void) { i8086_ICount-=9; EO=(WORD)(I.regs.w[BP]+(INT8)FETCHOP); EA=DefaultBase(SS)+EO; return EA; }
static unsigned EA_107(void) { i8086_ICount-=9; EO=(WORD)(I.regs.w[BX]+(INT8)FETCHOP); EA=DefaultBase(DS)+EO; return EA; }

static unsigned EA_200(void) { i8086_ICount-=11; EO=FETCHOP; EO+=FETCHOP<<8; EO+=I.regs.w[BX]+I.regs.w[SI]; EA=DefaultBase(DS)+(WORD)EO; return EA; }
static unsigned EA_201(void) { i8086_ICount-=12; EO=FETCHOP; EO+=FETCHOP<<8; EO+=I.regs.w[BX]+I.regs.w[DI]; EA=DefaultBase(DS)+(WORD)EO; return EA; }
static unsigned EA_202(void) { i8086_ICount-=12; EO=FETCHOP; EO+=FETCHOP<<8; EO+=I.regs.w[BP]+I.regs.w[SI]; EA=DefaultBase(SS)+(WORD)EO; return EA; }
static unsigned EA_203(void) { i8086_ICount-=11; EO=FETCHOP; EO+=FETCHOP<<8; EO+=I.regs.w[BP]+I.regs.w[DI]; EA=DefaultBase(SS)+(WORD)EO; return EA; }
static unsigned EA_204(void) { i8086_ICount-=9; EO=FETCHOP; EO+=FETCHOP<<8; EO+=I.regs.w[SI]; EA=DefaultBase(DS)+(WORD)EO; return EA; }
static unsigned EA_205(void) { i8086_ICount-=9; EO=FETCHOP; EO+=FETCHOP<<8; EO+=I.regs.w[DI]; EA=DefaultBase(DS)+(WORD)EO; return EA; }
static unsigned EA_206(void) { i8086_ICount-=9; EO=FETCHOP; EO+=FETCHOP<<8; EO+=I.regs.w[BP]; EA=DefaultBase(SS)+(WORD)EO; return EA; }
static unsigned EA_207(void) { i8086_ICount-=9; EO=FETCHOP; EO+=FETCHOP<<8; EO+=I.regs.w[BX]; EA=DefaultBase(DS)+(WORD)EO; return EA; }

static unsigned (*GetEA[192])(void)={
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
