#define ILLEGAL_INSTRUCTION 6
#define GENERAL_PROTECTION_FAULT 0xd

#define PM (I.msw&1)
#define CPL (I.sregs[CS]&3)
#define IOPL ((I.flags&0x3000)>>12)

static void i80286_trap2(int number);
static void i80286_interrupt_descriptor(UINT16 number);
static void i80286_code_descriptor(UINT16 selector, UINT16 offset);
static void i80286_data_descriptor(int reg, UINT16 selector);
static void PREFIX286(_0fpre)(void);
static void PREFIX286(_arpl)(void);
