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

#ifndef MAME_CPU_MCS51_MCS51_H
#define MAME_CPU_MCS51_MCS51_H

#pragma once


enum
{
	MCS51_PC=1, MCS51_SP, MCS51_PSW, MCS51_ACC, MCS51_B, MCS51_DPTR, MCS51_DPH, MCS51_DPL, MCS51_IE, MCS51_IP,
	MCS51_P0, MCS51_P1, MCS51_P2, MCS51_P3,
	MCS51_R0, MCS51_R1, MCS51_R2, MCS51_R3, MCS51_R4, MCS51_R5, MCS51_R6, MCS51_R7, MCS51_RB,
	MCS51_TCON, MCS51_TMOD, MCS51_TL0, MCS51_TL1, MCS51_TH0, MCS51_TH1
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


class mcs51_cpu_device : public cpu_device
{
public:
	/* At least CMOS devices may be forced to read from ports configured as output.
	 * All you need is a low impedance output connect to the port.
	 */
	void set_port_forced_input(uint8_t port, uint8_t forced_input) { m_forced_inputs[port] = forced_input; }

	template <unsigned N> auto port_in_cb() { return m_port_in_cb[N].bind(); }
	template <unsigned N> auto port_out_cb() { return m_port_out_cb[N].bind(); }
	auto serial_rx_cb() { return m_serial_rx_cb.bind(); }
	auto serial_tx_cb() { return m_serial_tx_cb.bind(); }

	void program_internal(address_map &map);
	void data_internal(address_map &map);
protected:
	// construction/destruction
	mcs51_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, int data_width, uint8_t features = 0);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 12 - 1) / 12; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 12); }
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 20; }
	virtual uint32_t execute_input_lines() const noexcept override { return 6; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

protected:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	//Internal stuff
	uint16_t  m_ppc;            //previous pc
	uint16_t  m_pc;             //current pc
	uint16_t  m_features;       //features of this cpu
	uint8_t   m_rwm;            //Signals that the current instruction is a read/write/modify instruction

	int     m_inst_cycles;        /* cycles for the current instruction */
	const uint32_t m_rom_size;    /* size (in bytes) of internal program ROM/EPROM */
	int     m_ram_mask;           /* second ram bank for indirect access available ? */
	int     m_num_interrupts;     /* number of interrupts supported */
	int     m_recalc_parity;      /* recalculate parity before next instruction */
	uint32_t  m_last_line_state;    /* last state of input lines line */
	int     m_t0_cnt;             /* number of 0->1 transitions on T0 line */
	int     m_t1_cnt;             /* number of 0->1 transitions on T1 line */
	int     m_t2_cnt;             /* number of 0->1 transitions on T2 line */
	int     m_t2ex_cnt;           /* number of 0->1 transitions on T2EX line */
	int     m_cur_irq_prio;       /* Holds value of the current IRQ Priority Level; -1 if no irq */
	uint8_t   m_irq_active;         /* mask which irq levels are serviced */
	uint8_t   m_irq_prio[8];        /* interrupt priority */

	uint8_t   m_forced_inputs[4];   /* allow read even if configured as output */

	// JB-related hacks
	uint8_t m_last_op;
	uint8_t m_last_bit;

	int     m_icount;

	struct mcs51_uart
	{
		uint8_t   data_out;       //Data to send out
		uint8_t   bits_to_send;   //How many bits left to send when transmitting out the serial port

		int     smod_div;       /* signal divided by 2^SMOD */
		int     rx_clk;         /* rx clock */
		int     tx_clk;         /* tx clock */
		uint8_t   delay_cycles;   //Gross Hack;
	} m_uart;            /* internal uart */

	/* Internal Ram */
	required_shared_ptr<uint8_t> m_sfr_ram;           /* 128 SFR - these are in 0x80 - 0xFF */
	required_shared_ptr<uint8_t> m_scratchpad;        /* 128 RAM (8031/51) + 128 RAM in second bank (8032/52) */

	/* SFR Callbacks */
	virtual void sfr_write(size_t offset, uint8_t data);
	virtual uint8_t sfr_read(size_t offset);

	/* Memory spaces */
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_program;
	memory_access< 9, 0, 0, ENDIANNESS_LITTLE>::specific m_data;
	memory_access<17, 0, 0, ENDIANNESS_LITTLE>::specific m_io;

	devcb_read8::array<4> m_port_in_cb;
	devcb_write8::array<4> m_port_out_cb;

	/* Serial Port TX/RX Callbacks */
	devcb_write8 m_serial_tx_cb;    //Call back function when sending data out of serial port
	devcb_read8 m_serial_rx_cb;    //Call back function to retrieve data when receiving serial port data

	/* DS5002FP */
	struct {
		uint8_t   previous_ta;        /* Previous Timed Access value */
		uint8_t   ta_window;          /* Limed Access window */
		uint8_t   range;              /* Memory Range */
		/* Bootstrap Configuration */
		uint8_t   mcon;                   /* bootstrap loader MCON register */
		uint8_t   rpctl;                  /* bootstrap loader RPCTL register */
		uint8_t   crc;                    /* bootstrap loader CRC register */
		int32_t   rnr_delay;              /* delay before new random number available */
	} m_ds5002fp;

	// for the debugger
	uint8_t m_rtemp;

	static const uint8_t mcs51_cycles[256];

	uint8_t iram_iread(offs_t a);
	void iram_iwrite(offs_t a, uint8_t d);
	void clear_current_irq();
	uint8_t r_acc();
	uint8_t r_psw();
	offs_t external_ram_iaddr(offs_t offset, offs_t mem_mask);
	uint8_t iram_read(size_t offset);
	void iram_write(size_t offset, uint8_t data);
	void push_pc();
	void pop_pc();
	void set_parity();
	uint8_t bit_address_r(uint8_t offset);
	void bit_address_w(uint8_t offset, uint8_t bit);
	void do_add_flags(uint8_t a, uint8_t data, uint8_t c);
	void do_sub_flags(uint8_t a, uint8_t data, uint8_t c);
	void transmit_receive(int source);
	void update_timer_t0(int cycles);
	void update_timer_t1(int cycles);
	void update_timer_t2(int cycles);
	void update_timers(int cycles);
	void serial_transmit(uint8_t data);
	void serial_receive();
	void update_serial(int cycles);
	void update_irq_prio(uint8_t ipl, uint8_t iph);
	void execute_op(uint8_t op);
	void check_irqs();
	void burn_cycles(int cycles);
	void acall(uint8_t r);
	void add_a_byte(uint8_t r);
	void add_a_mem(uint8_t r);
	void add_a_ir(uint8_t r);
	void add_a_r(uint8_t r);
	void addc_a_byte(uint8_t r);
	void addc_a_mem(uint8_t r);
	void addc_a_ir(uint8_t r);
	void addc_a_r(uint8_t r);
	void ajmp(uint8_t r);
	void anl_mem_a(uint8_t r);
	void anl_mem_byte(uint8_t r);
	void anl_a_byte(uint8_t r);
	void anl_a_mem(uint8_t r);
	void anl_a_ir(uint8_t r);
	void anl_a_r(uint8_t r);
	void anl_c_bitaddr(uint8_t r);
	void anl_c_nbitaddr(uint8_t r);
	void cjne_a_byte(uint8_t r);
	void cjne_a_mem(uint8_t r);
	void cjne_ir_byte(uint8_t r);
	void cjne_r_byte(uint8_t r);
	void clr_bitaddr(uint8_t r);
	void clr_c(uint8_t r);
	void clr_a(uint8_t r);
	void cpl_bitaddr(uint8_t r);
	void cpl_c(uint8_t r);
	void cpl_a(uint8_t r);
	void da_a(uint8_t r);
	void dec_a(uint8_t r);
	void dec_mem(uint8_t r);
	void dec_ir(uint8_t r);
	void dec_r(uint8_t r);
	void div_ab(uint8_t r);
	void djnz_mem(uint8_t r);
	void djnz_r(uint8_t r);
	void inc_a(uint8_t r);
	void inc_mem(uint8_t r);
	void inc_ir(uint8_t r);
	void inc_r(uint8_t r);
	void inc_dptr(uint8_t r);
	void jb(uint8_t r);
	void jbc(uint8_t r);
	void jc(uint8_t r);
	void jmp_iadptr(uint8_t r);
	void jnb(uint8_t r);
	void jnc(uint8_t r);
	void jnz(uint8_t r);
	void jz(uint8_t r);
	void lcall(uint8_t r);
	void ljmp(uint8_t r);
	void mov_a_byte(uint8_t r);
	void mov_a_mem(uint8_t r);
	void mov_a_ir(uint8_t r);
	void mov_a_r(uint8_t r);
	void mov_mem_byte(uint8_t r);
	void mov_mem_mem(uint8_t r);
	void mov_ir_byte(uint8_t r);
	void mov_r_byte(uint8_t r);
	void mov_mem_ir(uint8_t r);
	void mov_mem_r(uint8_t r);
	void mov_dptr_byte(uint8_t r);
	void mov_bitaddr_c(uint8_t r);
	void mov_ir_mem(uint8_t r);
	void mov_r_mem(uint8_t r);
	void mov_mem_a(uint8_t r);
	void mov_ir_a(uint8_t r);
	void mov_r_a(uint8_t r);
	void movc_a_iapc(uint8_t r);
	void mov_c_bitaddr(uint8_t r);
	void movc_a_iadptr(uint8_t r);
	void movx_a_idptr(uint8_t r);
	void movx_a_ir(uint8_t r);
	void movx_idptr_a(uint8_t r);
	void movx_ir_a(uint8_t r);
	void mul_ab(uint8_t r);
	void nop(uint8_t r);
	void orl_mem_a(uint8_t r);
	void orl_mem_byte(uint8_t r);
	void orl_a_byte(uint8_t r);
	void orl_a_mem(uint8_t r);
	void orl_a_ir(uint8_t r);
	void orl_a_r(uint8_t r);
	void orl_c_bitaddr(uint8_t r);
	void orl_c_nbitaddr(uint8_t r);
	void pop(uint8_t r);
	void push(uint8_t r);
	void ret(uint8_t r);
	void reti(uint8_t r);
	void rl_a(uint8_t r);
	void rlc_a(uint8_t r);
	void rr_a(uint8_t r);
	void rrc_a(uint8_t r);
	void setb_c(uint8_t r);
	void setb_bitaddr(uint8_t r);
	void sjmp(uint8_t r);
	void subb_a_byte(uint8_t r);
	void subb_a_mem(uint8_t r);
	void subb_a_ir(uint8_t r);
	void subb_a_r(uint8_t r);
	void swap_a(uint8_t r);
	void xch_a_mem(uint8_t r);
	void xch_a_ir(uint8_t r);
	void xch_a_r(uint8_t r);
	void xchd_a_ir(uint8_t r);
	void xrl_mem_a(uint8_t r);
	void xrl_mem_byte(uint8_t r);
	void xrl_a_byte(uint8_t r);
	void xrl_a_mem(uint8_t r);
	void xrl_a_ir(uint8_t r);
	void xrl_a_r(uint8_t r);
	void illegal(uint8_t r);
	uint8_t ds5002fp_protected(size_t offset, uint8_t data, uint8_t ta_mask, uint8_t mask);
};


/* variants with no internal rom and 128 byte internal memory */
DECLARE_DEVICE_TYPE(I8031, i8031_device)
/* variants with no internal rom and 256 byte internal memory */
DECLARE_DEVICE_TYPE(I8032, i8032_device)
/* variants 4k internal rom and 128 byte internal memory */
DECLARE_DEVICE_TYPE(I8051, i8051_device)
DECLARE_DEVICE_TYPE(I8751, i8751_device)
/* variants 8k internal rom and 128 byte internal memory (no 8052 features) */
DECLARE_DEVICE_TYPE(AM8753, am8753_device)
/* variants 8k internal rom and 256 byte internal memory and more registers */
DECLARE_DEVICE_TYPE(I8052, i8052_device)
DECLARE_DEVICE_TYPE(I8752, i8752_device)
/* cmos variants */
DECLARE_DEVICE_TYPE(I80C31, i80c31_device)
DECLARE_DEVICE_TYPE(I80C51, i80c51_device)
DECLARE_DEVICE_TYPE(I87C51, i87c51_device)
DECLARE_DEVICE_TYPE(I80C32, i80c32_device)
DECLARE_DEVICE_TYPE(I80C52, i80c52_device)
DECLARE_DEVICE_TYPE(I87C52, i87c52_device)
DECLARE_DEVICE_TYPE(I87C51FA, i87c51fa_device)
DECLARE_DEVICE_TYPE(I80C51GB, i80c51gb_device)
DECLARE_DEVICE_TYPE(AT89C52, at89c52_device)
DECLARE_DEVICE_TYPE(AT89S52, at89s52_device)
DECLARE_DEVICE_TYPE(DS80C320, ds80c320_device)
DECLARE_DEVICE_TYPE(SAB80C535, sab80c535_device)
/* 4k internal perom and 128 internal ram and 2 analog comparators */
DECLARE_DEVICE_TYPE(AT89C4051, at89c4051_device)

DECLARE_DEVICE_TYPE(I8344, i8344_device)
DECLARE_DEVICE_TYPE(I8744, i8744_device)

DECLARE_DEVICE_TYPE(DS5002FP, ds5002fp_device)


class i8031_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8031_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8051_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8051_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8751_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8751_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class am8753_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	am8753_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class i8052_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8052_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	i8052_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, int data_width, uint8_t features = 0);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	/* SFR Callbacks */
	virtual void sfr_write(size_t offset, uint8_t data) override;
	virtual uint8_t sfr_read(size_t offset) override;
};

class i8032_device : public i8052_device
{
public:
	// construction/destruction
	i8032_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8752_device : public i8052_device
{
public:
	// construction/destruction
	i8752_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i80c31_device : public i8052_device
{
public:
	// construction/destruction
	i80c31_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


class i80c51_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i80c51_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	i80c51_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, int data_width, uint8_t features = 0);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class i87c51_device : public i80c51_device
{
public:
	// construction/destruction
	i87c51_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class i80c52_device : public i8052_device
{
public:
	// construction/destruction
	i80c52_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	i80c52_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, int data_width, uint8_t features = 0);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	/* SFR Callbacks */
	virtual void sfr_write(size_t offset, uint8_t data) override;
	virtual uint8_t sfr_read(size_t offset) override;
};

class i80c32_device : public i80c52_device
{
public:
	// construction/destruction
	i80c32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i87c52_device : public i80c52_device
{
public:
	// construction/destruction
	i87c52_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i87c51fa_device : public i80c52_device
{
public:
	// construction/destruction
	i87c51fa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	i87c51fa_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, int data_width, uint8_t features = 0);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class i80c51gb_device : public i87c51fa_device
{
public:
	// construction/destruction
	i80c51gb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class at89c52_device : public i80c52_device
{
public:
	// construction/destruction
	at89c52_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class at89s52_device : public i80c52_device
{
public:
	// construction/destruction
	at89s52_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class at89c4051_device : public i80c51_device
{
public:
	// construction/destruction
	at89c4051_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ds80c320_device : public i80c52_device
{
public:
	// construction/destruction
	ds80c320_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class sab80c535_device : public i80c51_device
{
public:
	// construction/destruction
	sab80c535_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class i8344_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8344_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class i8744_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8744_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
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

/* these allow the default state of RAM to be set from a region */
#define DS5002FP_SET_MON( _mcon) \
	ROM_FILL( 0xc6, 1, _mcon)

#define DS5002FP_SET_RPCTL( _rpctl) \
	ROM_FILL( 0xd8, 1, _rpctl)

#define DS5002FP_SET_CRCR( _crcr) \
	ROM_FILL( 0xc1, 1, _crcr)


class ds5002fp_device : public mcs51_cpu_device, public device_nvram_interface
{
public:
	// construction/destruction
	ds5002fp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_mcon(uint8_t mcon) { m_ds5002fp.mcon = mcon; }
	void set_rpctl(uint8_t rpctl) { m_ds5002fp.rpctl = rpctl; }
	void set_crc(uint8_t crc) { m_ds5002fp.crc = crc; }

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read( util::read_stream &file ) override;
	virtual bool nvram_write( util::write_stream &file ) override;

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	/* SFR Callbacks */
	virtual void sfr_write(size_t offset, uint8_t data) override;
	virtual uint8_t sfr_read(size_t offset) override;

	uint8_t handle_rnr();
	bool is_rnr_ready();

private:
	optional_memory_region m_region;
};


#endif // MAME_CPU_MCS51_MCS51_H
