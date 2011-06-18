#define DIVIDE_BY_ZERO 0
#define SINGLE_STEP 1
#define NMI 2
#define BREAK 3
#define INTO_OVERFLOW 4
#define BOUND_OVERRUN 5
#define ILLEGAL_INSTRUCTION 6
#define FPU_UNAVAILABLE 7
#define DOUBLE_FAULT 8
#define FPU_SEG_OVERRUN 9
#define INVALID_TSS 10
#define SEG_NOT_PRESENT 11
#define STACK_FAULT 12
#define GENERAL_PROTECTION_FAULT 13

#define CPL (cpustate->sregs[CS]&3)
#define PM (cpustate->msw&1)

static void i80286_trap2(i80286_state *cpustate,UINT32 error);
static void i80286_interrupt_descriptor(i80286_state *cpustate,UINT16 number, int trap, int error);
static void i80286_code_descriptor(i80286_state *cpustate,UINT16 selector, UINT16 offset, int gate);
static void i80286_data_descriptor(i80286_state *cpustate,int reg, UINT16 selector);
static void PREFIX286(_0fpre)(i80286_state *cpustate);
static void PREFIX286(_arpl)(i80286_state *cpustate);
static void PREFIX286(_escape_7)(i8086_state *cpustate);
static void i80286_pop_seg(i80286_state *cpustate,int reg);
static void i80286_load_flags(i80286_state *cpustate, UINT16 flags, int cpl);

enum i80286_size
{
	I80286_BYTE = 1,
	I80286_WORD = 2
};

enum i80286_operation
{
	I80286_READ = 1,
	I80286_WRITE,
	I80286_EXECUTE
};

static void i80286_check_permission(i8086_state *cpustate, UINT8 check_seg, UINT32 offset, UINT16 size, i80286_operation operation);

static inline UINT32 GetMemAddr(i80286_state *cpustate, UINT8 seg, UINT32 off, UINT16 size, i80286_operation op) {
	seg = DefaultSeg(seg);
	if(PM) i80286_check_permission(cpustate, seg, off, size, op);
	return (cpustate->base[seg] + off) & AMASK;
}
