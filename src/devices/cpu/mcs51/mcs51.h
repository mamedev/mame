// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud
/*****************************************************************************
 *
 *   mcs51.h
 *   Portable MCS-51 Family Emulator
 *
 *   Chips in the family:
 *   8051 Product Line (8031,8051,8751)
 *   8052 Product Line (8032,8052,8752)
 *   8054 Product Line (8054)
 *   8058 Product Line (8058)
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *  This work is based on:
 *  #1) 'Intel(tm) MC51 Microcontroller Family Users Manual' and
 *  #2) 8051 simulator by Travis Marlatte
 *  #3) Portable UPI-41/8041/8741/8042/8742 emulator V0.1 by Juergen Buchmueller (MAME CORE)
 *
 * 2008, October, Couriersud
 * - Rewrite of timer, interrupt and serial code
 * - addition of CMOS features
 * - internal memory maps
 * - addition of new processor types
 * - full emulation of 8xCx2 processors
 *****************************************************************************/

#pragma once

#ifndef __MCS51_H__
#define __MCS51_H__


enum
{
	MCS51_PC=1, MCS51_SP, MCS51_PSW, MCS51_ACC, MCS51_B, MCS51_DPH, MCS51_DPL, MCS51_IE,
	MCS51_R0, MCS51_R1, MCS51_R2, MCS51_R3, MCS51_R4, MCS51_R5, MCS51_R6, MCS51_R7, MCS51_RB
};

enum
{
	MCS51_INT0_LINE = 0,    /* P3.2: External Interrupt 0 */
	MCS51_INT1_LINE,        /* P3.3: External Interrupt 1 */
	MCS51_RX_LINE,          /* P3.0: Serial Port Receive Line */
	MCS51_T0_LINE,          /* P3,4: Timer 0 External Input */
	MCS51_T1_LINE,          /* P3.5: Timer 1 External Input */
	MCS51_T2_LINE,          /* P1.0: Timer 2 External Input */
	MCS51_T2EX_LINE,        /* P1.1: Timer 2 Capture Reload Trigger */

	DS5002FP_PFI_LINE       /* DS5002FP Power fail interrupt */
};

/* special I/O space ports */

enum
{
	MCS51_PORT_P0   = 0x20000,
	MCS51_PORT_P1   = 0x20001,
	MCS51_PORT_P2   = 0x20002,
	MCS51_PORT_P3   = 0x20003,
	MCS51_PORT_TX   = 0x20004   /* P3.1 */
};

/* At least CMOS devices may be forced to read from ports configured as output.
 * All you need is a low impedance output connect to the port.
 */

#define MCFG_MCS51_PORT1_CONFIG(_forced_inputs) \
	mcs51_cpu_device::set_port_forced_input(*device, 1, _forced_inputs);
#define MCFG_MCS51_PORT2_CONFIG(_forced_inputs) \
	mcs51_cpu_device::set_port_forced_input(*device, 2, _forced_inputs);
#define MCFG_MCS51_PORT3_CONFIG(_forced_inputs) \
	mcs51_cpu_device::set_port_forced_input(*device, 3, _forced_inputs);

class mcs51_cpu_device : public cpu_device
{
public:
	// construction/destruction
	mcs51_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int program_width, int data_width, UINT8 features = 0);

	void i8051_set_serial_tx_callback(write8_delegate tx_func);
	void i8051_set_serial_rx_callback(read8_delegate rx_func);

	// configuration helpers
	static void set_port_forced_input(device_t &device, UINT8 port, UINT8 forced_input) { downcast<mcs51_cpu_device &>(device).m_forced_inputs[port] = forced_input; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks + 12 - 1) / 12; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 12); }
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 20; }
	virtual UINT32 execute_input_lines() const override { return 6; }
	virtual UINT32 execute_default_irq_vector() const override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override
	{
		return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : ( (spacenum == AS_DATA) ? &m_data_config : nullptr ) );
	}

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 5; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

protected:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	//Internal stuff
	UINT16  m_ppc;            //previous pc
	UINT16  m_pc;             //current pc
	UINT16  m_features;       //features of this cpu
	UINT8   m_rwm;            //Signals that the current instruction is a read/write/modify instruction

	int     m_inst_cycles;        /* cycles for the current instruction */
	int     m_ram_mask;           /* second ram bank for indirect access available ? */
	int     m_num_interrupts;     /* number of interrupts supported */
	int     m_recalc_parity;      /* recalculate parity before next instruction */
	UINT32  m_last_line_state;    /* last state of input lines line */
	int     m_t0_cnt;             /* number of 0->1 transistions on T0 line */
	int     m_t1_cnt;             /* number of 0->1 transistions on T1 line */
	int     m_t2_cnt;             /* number of 0->1 transistions on T2 line */
	int     m_t2ex_cnt;           /* number of 0->1 transistions on T2EX line */
	int     m_cur_irq_prio;       /* Holds value of the current IRQ Priority Level; -1 if no irq */
	UINT8   m_irq_active;         /* mask which irq levels are serviced */
	UINT8   m_irq_prio[8];        /* interrupt priority */

	UINT8   m_forced_inputs[4];   /* allow read even if configured as output */

	int     m_icount;

	struct mcs51_uart
	{
		UINT8   data_out;       //Data to send out
		UINT8   bits_to_send;   //How many bits left to send when transmitting out the serial port

		int     smod_div;       /* signal divided by 2^SMOD */
		int     rx_clk;         /* rx clock */
		int     tx_clk;         /* tx clock */
		UINT8   delay_cycles;   //Gross Hack;
	} m_uart;            /* internal uart */

	/* Internal Ram */
	UINT8   *m_internal_ram;      /* 128 RAM (8031/51) + 128 RAM in second bank (8032/52) */
	UINT8   *m_sfr_ram;           /* 128 SFR - these are in 0x80 - 0xFF */

	/* SFR Callbacks */
	virtual void sfr_write(size_t offset, UINT8 data);
	virtual UINT8 sfr_read(size_t offset);

	/* Memory spaces */
	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	address_space *m_io;

	/* Serial Port TX/RX Callbacks */
	// TODO: Move to special port r/w
	write8_delegate m_serial_tx_callback;    //Call back funciton when sending data out of serial port
	read8_delegate m_serial_rx_callback;    //Call back function to retrieve data when receiving serial port data

	/* DS5002FP */
	struct {
		UINT8   previous_ta;        /* Previous Timed Access value */
		UINT8   ta_window;          /* Limed Access window */
		UINT8   range;              /* Memory Range */
		/* Bootstrap Configuration */
		UINT8   mcon;                   /* bootstrap loader MCON register */
		UINT8   rpctl;                  /* bootstrap loader RPCTL register */
		UINT8   crc;                    /* bootstrap loader CRC register */
	} m_ds5002fp;

	// for the debugger
	UINT8 m_rtemp;

	static const UINT8 mcs51_cycles[256];

	UINT8 iram_iread(offs_t a);
	void iram_iwrite(offs_t a, UINT8 d);
	void clear_current_irq();
	UINT8 r_acc();
	UINT8 r_psw();
	void update_ptrs();
	offs_t external_ram_iaddr(offs_t offset, offs_t mem_mask);
	UINT8 iram_read(size_t offset);
	void iram_write(size_t offset, UINT8 data);
	void push_pc();
	void pop_pc();
	void set_parity();
	UINT8 bit_address_r(UINT8 offset);
	void bit_address_w(UINT8 offset, UINT8 bit);
	void do_add_flags(UINT8 a, UINT8 data, UINT8 c);
	void do_sub_flags(UINT8 a, UINT8 data, UINT8 c);
	void transmit_receive(int source);
	void update_timer_t0(int cycles);
	void update_timer_t1(int cycles);
	void update_timer_t2(int cycles);
	void update_timers(int cycles);
	void serial_transmit(UINT8 data);
	void serial_receive();
	void update_serial(int cycles);
	void update_irq_prio(UINT8 ipl, UINT8 iph);
	void execute_op(UINT8 op);
	void check_irqs();
	void burn_cycles(int cycles);
	void acall(UINT8 r);
	void add_a_byte(UINT8 r);
	void add_a_mem(UINT8 r);
	void add_a_ir(UINT8 r);
	void add_a_r(UINT8 r);
	void addc_a_byte(UINT8 r);
	void addc_a_mem(UINT8 r);
	void addc_a_ir(UINT8 r);
	void addc_a_r(UINT8 r);
	void ajmp(UINT8 r);
	void anl_mem_a(UINT8 r);
	void anl_mem_byte(UINT8 r);
	void anl_a_byte(UINT8 r);
	void anl_a_mem(UINT8 r);
	void anl_a_ir(UINT8 r);
	void anl_a_r(UINT8 r);
	void anl_c_bitaddr(UINT8 r);
	void anl_c_nbitaddr(UINT8 r);
	void cjne_a_byte(UINT8 r);
	void cjne_a_mem(UINT8 r);
	void cjne_ir_byte(UINT8 r);
	void cjne_r_byte(UINT8 r);
	void clr_bitaddr(UINT8 r);
	void clr_c(UINT8 r);
	void clr_a(UINT8 r);
	void cpl_bitaddr(UINT8 r);
	void cpl_c(UINT8 r);
	void cpl_a(UINT8 r);
	void da_a(UINT8 r);
	void dec_a(UINT8 r);
	void dec_mem(UINT8 r);
	void dec_ir(UINT8 r);
	void dec_r(UINT8 r);
	void div_ab(UINT8 r);
	void djnz_mem(UINT8 r);
	void djnz_r(UINT8 r);
	void inc_a(UINT8 r);
	void inc_mem(UINT8 r);
	void inc_ir(UINT8 r);
	void inc_r(UINT8 r);
	void inc_dptr(UINT8 r);
	void jb(UINT8 r);
	void jbc(UINT8 r);
	void jc(UINT8 r);
	void jmp_iadptr(UINT8 r);
	void jnb(UINT8 r);
	void jnc(UINT8 r);
	void jnz(UINT8 r);
	void jz(UINT8 r);
	void lcall(UINT8 r);
	void ljmp(UINT8 r);
	void mov_a_byte(UINT8 r);
	void mov_a_mem(UINT8 r);
	void mov_a_ir(UINT8 r);
	void mov_a_r(UINT8 r);
	void mov_mem_byte(UINT8 r);
	void mov_mem_mem(UINT8 r);
	void mov_ir_byte(UINT8 r);
	void mov_r_byte(UINT8 r);
	void mov_mem_ir(UINT8 r);
	void mov_mem_r(UINT8 r);
	void mov_dptr_byte(UINT8 r);
	void mov_bitaddr_c(UINT8 r);
	void mov_ir_mem(UINT8 r);
	void mov_r_mem(UINT8 r);
	void mov_mem_a(UINT8 r);
	void mov_ir_a(UINT8 r);
	void mov_r_a(UINT8 r);
	void movc_a_iapc(UINT8 r);
	void mov_c_bitaddr(UINT8 r);
	void movc_a_iadptr(UINT8 r);
	void movx_a_idptr(UINT8 r);
	void movx_a_ir(UINT8 r);
	void movx_idptr_a(UINT8 r);
	void movx_ir_a(UINT8 r);
	void mul_ab(UINT8 r);
	void nop(UINT8 r);
	void orl_mem_a(UINT8 r);
	void orl_mem_byte(UINT8 r);
	void orl_a_byte(UINT8 r);
	void orl_a_mem(UINT8 r);
	void orl_a_ir(UINT8 r);
	void orl_a_r(UINT8 r);
	void orl_c_bitaddr(UINT8 r);
	void orl_c_nbitaddr(UINT8 r);
	void pop(UINT8 r);
	void push(UINT8 r);
	void ret(UINT8 r);
	void reti(UINT8 r);
	void rl_a(UINT8 r);
	void rlc_a(UINT8 r);
	void rr_a(UINT8 r);
	void rrc_a(UINT8 r);
	void setb_c(UINT8 r);
	void setb_bitaddr(UINT8 r);
	void sjmp(UINT8 r);
	void subb_a_byte(UINT8 r);
	void subb_a_mem(UINT8 r);
	void subb_a_ir(UINT8 r);
	void subb_a_r(UINT8 r);
	void swap_a(UINT8 r);
	void xch_a_mem(UINT8 r);
	void xch_a_ir(UINT8 r);
	void xch_a_r(UINT8 r);
	void xchd_a_ir(UINT8 r);
	void xrl_mem_a(UINT8 r);
	void xrl_mem_byte(UINT8 r);
	void xrl_a_byte(UINT8 r);
	void xrl_a_mem(UINT8 r);
	void xrl_a_ir(UINT8 r);
	void xrl_a_r(UINT8 r);
	void illegal(UINT8 r);
	UINT8 ds5002fp_protected(size_t offset, UINT8 data, UINT8 ta_mask, UINT8 mask);

};


/* variants with no internal rom and 128 byte internal memory */
extern const device_type I8031;
/* variants with no internal rom and 256 byte internal memory */
extern const device_type I8032;
/* variants 4k internal rom and 128 byte internal memory */
extern const device_type I8051;
extern const device_type I8751;
/* variants 8k internal rom and 256 byte internal memory and more registers */
extern const device_type I8052;
extern const device_type I8752;
/* cmos variants */
extern const device_type I80C31;
extern const device_type I80C51;
extern const device_type I87C51;
extern const device_type I80C32;
extern const device_type I80C52;
extern const device_type I87C52;
/* 4k internal perom and 128 internal ram and 2 analog comparators */
extern const device_type AT89C4051;

extern const device_type DS5002FP;


class i8031_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8031_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8051_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8051_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8751_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8751_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class i8052_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8052_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	i8052_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int program_width, int data_width, UINT8 features = 0);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	/* SFR Callbacks */
	virtual void sfr_write(size_t offset, UINT8 data) override;
	virtual UINT8 sfr_read(size_t offset) override;
};

class i8032_device : public i8052_device
{
public:
	// construction/destruction
	i8032_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8752_device : public i8052_device
{
public:
	// construction/destruction
	i8752_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i80c31_device : public i8052_device
{
public:
	// construction/destruction
	i80c31_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
};


class i80c51_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i80c51_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	i80c51_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int program_width, int data_width, UINT8 features = 0);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
};

class i87c51_device : public i80c51_device
{
public:
	// construction/destruction
	i87c51_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class i80c52_device : public i8052_device
{
public:
	// construction/destruction
	i80c52_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	i80c52_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int program_width, int data_width, UINT8 features = 0);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	/* SFR Callbacks */
	virtual void sfr_write(size_t offset, UINT8 data) override;
	virtual UINT8 sfr_read(size_t offset) override;
};

class i80c32_device : public i80c52_device
{
public:
	// construction/destruction
	i80c32_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i87c52_device : public i80c52_device
{
public:
	// construction/destruction
	i87c52_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class at89c4051_device : public i80c51_device
{
public:
	// construction/destruction
	at89c4051_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

/*
 * The DS5002FP has 2 16 bits data address buses (the byte-wide bus and the expanded bus). The exact memory position accessed depends on the
 * partition mode, the memory range and the expanded bus select. The partition mode and the expanded bus select can be changed at any time.
 *
 * In order to simplify memory mapping to the data address bus, the following address map is assumed for partitioned mode:

 * 0x00000-0x0ffff -> data memory on the expanded bus
 * 0x10000-0x1ffff -> data memory on the byte-wide bus

 * For non-partitioned mode the following memory map is assumed:

 * 0x0000-0xffff -> data memory (the bus used to access it does not matter)
 *
 * Internal ram 128k and security features
 */

#define MCFG_DS5002FP_CONFIG(_mcon, _rpctl, _crc) \
	ds5002fp_device::set_mcon(*device, _mcon); \
	ds5002fp_device::set_rpctl(*device, _rpctl); \
	ds5002fp_device::set_crc(*device, _crc);

class ds5002fp_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	ds5002fp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_mcon(device_t &device, UINT8 mcon) { downcast<ds5002fp_device &>(device).m_ds5002fp.mcon = mcon; }
	static void set_rpctl(device_t &device, UINT8 rpctl) { downcast<ds5002fp_device &>(device).m_ds5002fp.rpctl = rpctl; }
	static void set_crc(device_t &device, UINT8 crc) { downcast<ds5002fp_device &>(device).m_ds5002fp.crc = crc; }

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	/* SFR Callbacks */
	virtual void sfr_write(size_t offset, UINT8 data) override;
	virtual UINT8 sfr_read(size_t offset) override;
};


#endif /* __MCS51_H__ */
