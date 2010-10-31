#define ILLEGAL_INSTRUCTION 6
#define GENERAL_PROTECTION_FAULT 0xd

#define PM (cpustate->msw&1)
#define CPL (cpustate->sregs[CS]&3)
#define IOPL ((cpustate->flags&0x3000)>>12)

static void i80286_trap2(i80286_state *cpustate,int number);
static void i80286_interrupt_descriptor(i80286_state *cpustate,UINT16 number);
static void i80286_code_descriptor(i80286_state *cpustate,UINT16 selector, UINT16 offset);
static void i80286_data_descriptor(i80286_state *cpustate,int reg, UINT16 selector);
static void PREFIX286(_0fpre)(i80286_state *cpustate);
static void PREFIX286(_arpl)(i80286_state *cpustate);

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
