/*
    Panasonic MN10200 emulator

    Written by Olivier Galibert
    MAME conversion by R. Belmont

*/

#pragma once

#ifndef MN10200_H
#define MN10200_H

enum
{
	MN10200_PC = 0,
	MN10200_PSW,
	MN10200_MDR,
	MN10200_D0,
	MN10200_D1,
	MN10200_D2,
	MN10200_D3,
	MN10200_A0,
	MN10200_A1,
	MN10200_A2,
	MN10200_A3,
	MN10200_NMICR,
	MN10200_IAGR
};

enum
{
	MN10200_PORT0 = 0,
	MN10200_PORT1,
	MN10200_PORT2,
	MN10200_PORT3,
	MN10200_PORT4
};

enum
{
	MN10200_IRQ0 = 0,
	MN10200_IRQ1,
	MN10200_IRQ2,
	MN10200_IRQ3
};


extern const device_type MN10200;


#define MN10200_NUM_PRESCALERS (2)
#define MN10200_NUM_TIMERS_8BIT (10)
#define MN10200_NUM_IRQ_GROUPS (31)


class mn10200_device : public cpu_device
{
public:
	// construction/destruction
	mn10200_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 8; }
	virtual UINT32 execute_input_lines() const { return 4; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const
	{
		return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : NULL );
	}

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, astring &string);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
	address_space_config m_program_config;
	address_space_config m_io_config;

	// The UINT32s are really UINT24
	UINT32 m_pc;
	UINT32 m_d[4];
	UINT32 m_a[4];

	UINT8 m_nmicr;
	UINT8 m_iagr;
	UINT8 m_icrl[MN10200_NUM_IRQ_GROUPS];
	UINT8 m_icrh[MN10200_NUM_IRQ_GROUPS];
	UINT16 m_psw;
	UINT16 m_mdr;

	struct {
		UINT8 mode;
		UINT8 base;
		UINT8 cur;
	} m_simple_timer[MN10200_NUM_TIMERS_8BIT];

	emu_timer *m_timer_timers[MN10200_NUM_TIMERS_8BIT];

	struct {
		UINT8 cycles;
		UINT8 mode;
	} m_prescaler[MN10200_NUM_PRESCALERS];

	struct {
		UINT32 adr;
		UINT32 count;
		UINT16 iadr;
		UINT8 ctrll;
		UINT8 ctrlh;
		UINT8 irq;
	} m_dma[8];

	struct {
		UINT8 ctrll;
		UINT8 ctrlh;
		UINT8 buf;
	} m_serial[2];

	UINT8 m_ddr[8];

	int m_cycles;

	address_space *m_program;
	address_space *m_io;

	UINT8 mn102_read_byte(UINT32 address);
	UINT16 mn102_read_word(UINT32 address);
	void mn102_write_byte(UINT32 address, UINT8 data);
	void mn102_write_word(UINT32 address, UINT16 data);
	INT32 r24u(offs_t adr);
	void w24(offs_t adr, UINT32 val);
	void mn102_change_pc(UINT32 pc);
	void mn102_take_irq(int level, int group);
	void refresh_timer(int tmr);
	void timer_tick_simple(int tmr);
	TIMER_CALLBACK_MEMBER( simple_timer_cb );
	void unemul();
	UINT32 do_add(UINT32 a, UINT32 b);
	UINT32 do_addc(UINT32 a, UINT32 b);
	UINT32 do_sub(UINT32 a, UINT32 b);
	UINT32 do_subc(UINT32 a, UINT32 b);
	void test_nz16(UINT16 v);
	void do_jsr(UINT32 to, UINT32 ret);
	void mn10200_w(UINT32 adr, UINT32 data, int type);
	UINT32 mn10200_r(UINT32 adr, int type);
};


#endif
