#ifndef HEADER__M37710
#define HEADER__M37710

/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

M37710 CPU Emulator v0.1

*/

/* ======================================================================== */
/* =============================== DEFINES ================================ */
/* ======================================================================== */
/*
   Input lines - used with cpunum_set_input_line() and the like.
   WARNING: these are in the same order as the vector table for simplicity.
   Do not alter this order!
*/

enum
{
	// these interrupts are maskable
	M37710_LINE_ADC = 0,
	M37710_LINE_UART1XMIT,
	M37710_LINE_UART1RECV,
	M37710_LINE_UART0XMIT,
	M37710_LINE_UART0RECV,
	M37710_LINE_TIMERB2,
	M37710_LINE_TIMERB1,
	M37710_LINE_TIMERB0,
	M37710_LINE_TIMERA4,
	M37710_LINE_TIMERA3,
	M37710_LINE_TIMERA2,
	M37710_LINE_TIMERA1,
	M37710_LINE_TIMERA0,
	M37710_LINE_IRQ2,
	M37710_LINE_IRQ1,
	M37710_LINE_IRQ0,
	// these interrupts are non-maskable
	M37710_LINE_WATCHDOG,
	M37710_LINE_DEBUG,
	M37710_LINE_BRK,
	M37710_LINE_ZERODIV,
	M37710_LINE_RESET,

	// these are not interrupts, they're signals external hardware can send
	M37710_LINE_TIMERA0TICK,
	M37710_LINE_TIMERA1TICK,
	M37710_LINE_TIMERA2TICK,
	M37710_LINE_TIMERA3TICK,
	M37710_LINE_TIMERA4TICK,
	M37710_LINE_TIMERB0TICK,
	M37710_LINE_TIMERB1TICK,
	M37710_LINE_TIMERB2TICK,

	M37710_LINE_MAX
};

/* Registers - used by m37710_set_reg() and m37710_get_reg() */
enum
{
	M37710_PC=1, M37710_S, M37710_P, M37710_A, M37710_B, M37710_X, M37710_Y,
	M37710_PB, M37710_DB, M37710_D, M37710_E,
	M37710_NMI_STATE, M37710_IRQ_STATE
};


/* I/O ports */
enum
{
	M37710_PORT0 = 0,
	M37710_PORT1, M37710_PORT2, M37710_PORT3, M37710_PORT4,
	M37710_PORT5, M37710_PORT6, M37710_PORT7, M37710_PORT8,

	M37710_ADC0_L = 0x10, M37710_ADC0_H,
	M37710_ADC1_L, M37710_ADC1_H, M37710_ADC2_L, M37710_ADC2_H, M37710_ADC3_L, M37710_ADC3_H,
	M37710_ADC4_L, M37710_ADC4_H, M37710_ADC5_L, M37710_ADC5_H, M37710_ADC6_L, M37710_ADC6_H,
	M37710_ADC7_L, M37710_ADC7_H,

	M37710_SER0_REC = 0x20,
	M37710_SER0_XMIT, M37710_SER1_REC, M37710_SER1_XMIT
};

/* ======================================================================== */
/* ============================== PROTOTYPES ============================== */
/* ======================================================================== */

extern int m37710_ICount;				/* cycle count */

/* ======================================================================== */
/* ================================= MAME ================================= */
/* ======================================================================== */

/* Clean up after the emulation core - Not used in this core - */
void m37710_exit(void);

/* Save the current CPU state to disk */
void m37710_state_save(void *file);

/* Load a CPU state from disk */
void m37710_state_load(void *file);

#undef M37710_CALL_DEBUGGER

#define M37710_CALL_DEBUGGER CALL_MAME_DEBUG
#define m37710_read_8(addr) 			program_read_byte_16le(addr)
#define m37710_write_8(addr,data)		program_write_byte_16le(addr,data)
#define m37710_read_8_immediate(A)		program_read_byte_16le(A)
#define m37710_read_16(addr) 			program_read_word_16le(addr)
#define m37710_write_16(addr,data)		program_write_word_16le(addr,data)
#define m37710_jumping(A)			change_pc(A)
#define m37710_branching(A)


void m37710_get_info(UINT32 state, cpuinfo *info);

/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* HEADER__M37710 */
