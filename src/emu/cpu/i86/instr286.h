#define DIVIDE_BY_ZERO 0
#define SINGLE_STEP 1
#define NMI 2
#define BREAK 3
#define INTO_OVERFLOW 4
#define BOUND_OVERRUN 5
#define ILLEGAL_INSTRUCTION 6
#define CPU_EXT_UNAVAILABLE 7
#define DOUBLE_FAULT 8
#define CPU_EXT_SEG_OVERRUN 9
#define INVALID_TSS 10
#define SEG_NOT_PRESENT 11
#define STACK_FAULT 12
#define GENERAL_PROTECTION_FAULT 13

#define PM (cpustate->msw&1)
#define CPL (cpustate->sregs[CS]&3)

static void i80286_trap2(i80286_state *cpustate,UINT32 error);
static void i80286_interrupt_descriptor(i80286_state *cpustate,UINT16 number, int trap, int error);
static void i80286_code_descriptor(i80286_state *cpustate,UINT16 selector, UINT16 offset, int gate);
static void i80286_data_descriptor(i80286_state *cpustate,int reg, UINT16 selector);
static void PREFIX286(_0fpre)(i80286_state *cpustate);
static void PREFIX286(_arpl)(i80286_state *cpustate);
static void i80286_pop_seg(i80286_state *cpustate,int reg);

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

static void i80286_check_permission(i8086_state *cpustate, UINT8 check_seg, UINT16 offset, i80286_size size, i80286_operation operation);
