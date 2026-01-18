// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud
/*****************************************************************************
 *
 * Portable MCS-51 Family Emulator
 * Copyright Steve Ellenoff
 *
 *****************************************************************************/

#ifndef MAME_CPU_MCS51_I8051_H
#define MAME_CPU_MCS51_I8051_H

#pragma once


enum
{
	MCS51_PC=1, MCS51_SP, MCS51_PSW, MCS51_ACC, MCS51_B, MCS51_DPTR, MCS51_IE, MCS51_IP,
	MCS51_P0, MCS51_P1, MCS51_P2, MCS51_P3,
	MCS51_R0, MCS51_R1, MCS51_R2, MCS51_R3, MCS51_R4, MCS51_R5, MCS51_R6, MCS51_R7, MCS51_RB,
	MCS51_TCON, MCS51_TMOD, MCS51_TL0, MCS51_TL1, MCS51_TH0, MCS51_TH1
};

enum
{
	MCS51_INT0_LINE = 0,    // P3.2: External Interrupt 0
	MCS51_INT1_LINE,        // P3.3: External Interrupt 1
	MCS51_T0_LINE,          // P3.4: Timer 0 External Input
	MCS51_T1_LINE,          // P3.5: Timer 1 External Input
	MCS51_T2_LINE,          // P1.0: Timer 2 External Input
	MCS51_T2EX_LINE,        // P1.1: Timer 2 Capture Reload Trigger

	DS5002FP_PFI_LINE       // DS5002FP Power fail interrupt
};


class mcs51_cpu_device : public cpu_device
{
public:
	/* At least CMOS devices may be forced to read from ports configured as output.
	 * All you need is a low impedance output connect to the port.
	 */
	void set_port_forced_input(u8 port, u8 forced_input) { m_forced_inputs[port] = forced_input; }

	template <unsigned N> auto port_in_cb() { return m_port_in_cb[N].bind(); }
	template <unsigned N> auto port_out_cb() { return m_port_out_cb[N].bind(); }

protected:
	enum {
		AS_INTD = 4,  // Direct internal access space
		AS_INTI = 5,  // Indirect internal access space
	};

	enum {
		PSW_CY = 7,  //Carry Flag
		PSW_AC = 6,  //Aux.Carry Flag
		PSW_FO = 5,  //User Flag
		PSW_RS = 3,  //R Bank Select
		PSW_OV = 2,  //Overflow Flag
		PSW_P  = 0,  //Parity Flag
	};

	enum {
		IE_A  = 7,  //Global Interrupt Enable/Disable
		IE_T2 = 6,  //Timer 2 Interrupt Enable/Disable
		IE_S  = 4,  //Serial Interrupt Enable/Disable
		IE_T1 = 3,  //Timer 1 Interrupt Enable/Disable
		IE_X1 = 2,  //External Int 1 Interrupt Enable/Disable
		IE_T0 = 1,  //Timer 0 Interrupt Enable/Disable
		IE_X0 = 0,  //External Int 0 Interrupt Enable/Disable
	};

	enum {
		IP_T2 = 5,  //Set Timer 2 Priority Level
		IP_S  = 4,  //Set Serial Priority Level
		IP_T1 = 3,  //Set Timer 1 Priority Level
		IP_X1 = 2,  //Set External Int 1 Priority Level
		IP_T0 = 1,  //Set Timer 0 Priority Level
		IP_X0 = 0,  //Set External Int 0 Priority Level
	};

	enum {
		TCON_TF1 = 7,  //Timer 1 Overflow Int Triggered
		TCON_TR1 = 6,  //Timer 1 is running
		TCON_TF0 = 5,  //Timer 0 Overflow Int Triggered
		TCON_TR0 = 4,  //Timer 0 is running
		TCON_IE1 = 3,  //External Int 1 Triggered
		TCON_IT1 = 2,  //External Int 1 Trigger Cause
		TCON_IE0 = 1,  //External Int 0 Triggered
		TCON_IT0 = 0,  //External Int 0 Trigger Cause
	};

	enum {
		SCON_SM0 = 7,  //Sets Serial Port Mode
		SCON_SM1 = 6,  //Sets Serial Port Mode
		SCON_SM2 = 5,  //Sets Serial Port Mode (Multiprocesser mode)
		SCON_REN = 4,  //Sets Serial Port Receive Enable
		SCON_TB8 = 3,  //Transmit 8th Bit
		SCON_RB8 = 2,  //Receive 8th Bit
		SCON_TI  = 1,  //Indicates Transmit Interrupt Occurred
		SCON_RI  = 0,  //Indicates Receive Interrupt Occurred
	};

	enum {
		TMOD_GATE1 = 7,  //Timer 1 Gate Mode
		TMOD_CT1   = 6,  //Timer 1 Counter Mode
		TMOD_M1_1  = 5,  //Timer 1 Timer Mode Bit 1
		TMOD_M1_0  = 4,  //Timer 1 Timer Mode Bit 0
		TMOD_GATE0 = 3,  //Timer 0 Gate Mode
		TMOD_CT0   = 2,  //Timer 0 Counter Mode
		TMOD_M0_1  = 1,  //Timer 0 Timer Mode Bit 1
		TMOD_M0_0  = 0,  //Timer 0 Timer Mode Bit 0
	};

	enum {
		PCON_SMOD  = 7,
		PCON_GF1   = 3,
		PCON_GF0   = 2,
		PCON_PD    = 1,
		PCON_IDL   = 0,

		PCON_POR   = 6,
		PCON_PFW   = 5,
		PCON_WTR   = 4,
		PCON_EPFW  = 3,
		PCON_EWT   = 2,
	};

	// construction/destruction
	mcs51_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, int io_width);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 12 - 1) / 12; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 12); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 4+2; } // max opcode cycles + irq
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_intd_config;
	address_space_config m_inti_config;

	// Internal stuff
	u16 m_ppc;              // previous pc
	u16 m_pc;               // current pc
	bool m_has_pd;          // can powerdown
	u8  m_rwm;              // Signals that the current instruction is a read/write/modify instruction

	int      m_inst_cycles;      // cycles for the current instruction
	const u32 m_rom_size;   // size (in bytes) of internal program ROM/EPROM
	int      m_ram_mask;         // second ram bank for indirect access available ?
	int      m_num_interrupts;   // number of interrupts supported
	u32 m_last_line_state;  // last state of input lines line
	int      m_t0_cnt;           // number of 0->1 transitions on T0 line
	int      m_t1_cnt;           // number of 0->1 transitions on T1 line
	int      m_t2_cnt;           // number of 0->1 transitions on T2 line
	int      m_t2ex_cnt;         // number of 0->1 transitions on T2EX line
	int      m_cur_irq_prio;     // Holds value of the current IRQ Priority Level; -1 if no irq
	u8  m_irq_active;       // mask which irq levels are serviced
	u8  m_irq_prio[8];      // interrupt priority

	u8  m_forced_inputs[4]; // allow read even if configured as output

	// JB-related hacks
	u8  m_last_op;
	u8  m_last_bit;

	int      m_icount;

	struct mcs51_uart
	{
		u8  data_out;       // data to send out
		u8  data_in;
		u8  txbit;
		u8  txd;
		u8  rxbit;
		u8  rxb8;

		int      smod_div;       // signal divided by 2^SMOD
		int      rx_clk;         // rx clock
		int      tx_clk;         // tx clock
	} m_uart;                    // internal uart

	// Internal Ram
	u16 m_dptr;
	u8 m_acc, m_psw, m_b, m_sp, m_pcon, m_tcon, m_tmod, m_scon, m_sbuf, m_ie, m_ip, m_iph;
	u8 m_p0, m_p1, m_p2, m_p3;
	u8 m_tl0, m_tl1, m_th0, m_th1;

	required_shared_ptr<u8> m_internal_ram; // 128 RAM (8031/51) + 128 RAM in second bank (8032/52)

	void program_map(address_map &map) ATTR_COLD;
	void intd_map(address_map &map) ATTR_COLD;
	void inti_map(address_map &map) ATTR_COLD;
	virtual void sfr_map(address_map &map) ATTR_COLD;

	// SFR Callbacks
	u8   psw_r  ();
	void psw_w  (u8 data);
	u8   acc_r  ();
	void acc_w  (u8 data);
	u8   b_r    ();
	void b_w    (u8 data);

	u8   sp_r   ();
	void sp_w   (u8 data);
	u8   dptr_r (offs_t offset);
	void dptr_w (offs_t offset, u8 data);
	u8   pcon_r ();
	void pcon_w (u8 data);
	u8   tcon_r ();
	void tcon_w (u8 data);
	u8   tmod_r ();
	void tmod_w (u8 data);
	u8   scon_r ();
	void scon_w (u8 data);
	u8   sbuf_r ();
	void sbuf_w (u8 data);
	u8   ie_r   ();
	void ie_w   (u8 data);
	u8   ip_r   ();
	void ip_w   (u8 data);

	u8   p0_r   ();
	void p0_w   (u8 data);
	u8   p1_r   ();
	void p1_w   (u8 data);
	u8   p2_r   ();
	void p2_w   (u8 data);
	u8   p3_r   ();
	void p3_w   (u8 data);

	u8   tl0_r  ();
	void tl0_w  (u8 data);
	u8   tl1_r  ();
	void tl1_w  (u8 data);
	u8   th0_r  ();
	void th0_w  (u8 data);
	u8   th1_r  ();
	void th1_w  (u8 data);

	template<int bit> void set_bit(u8 &reg, bool state) {
		if (state)
			reg |= 1 << bit;
		else
			reg &= ~(1 << bit);
	}

	void set_cy (bool state) { set_bit<PSW_CY>(m_psw, state); }
	void set_ac (bool state) { set_bit<PSW_AC>(m_psw, state); }
	void set_fo (bool state) { set_bit<PSW_FO>(m_psw, state); }
	void set_rs (u8 state)   { m_psw = (m_psw & ~(3 << PSW_RS)) | (state << PSW_RS); }
	void set_ov (bool state) { set_bit<PSW_OV>(m_psw, state); }
	void set_p  (bool state) { set_bit<PSW_P >(m_psw, state); }

	void set_tf1(bool state) { set_bit<TCON_TF1>(m_tcon, state); }
	void set_tr1(bool state) { set_bit<TCON_TR1>(m_tcon, state); }
	void set_tf0(bool state) { set_bit<TCON_TF0>(m_tcon, state); }
	void set_tr0(bool state) { set_bit<TCON_TR0>(m_tcon, state); }
	void set_ie1(bool state) { set_bit<TCON_IE1>(m_tcon, state); }
	void set_it1(bool state) { set_bit<TCON_IT1>(m_tcon, state); }
	void set_ie0(bool state) { set_bit<TCON_IE0>(m_tcon, state); }
	void set_it0(bool state) { set_bit<TCON_IT0>(m_tcon, state); }

	void set_rb8(bool state) { set_bit<SCON_RB8>(m_scon, state); }
	void set_ti (bool state) { set_bit<SCON_TI >(m_scon, state); }
	void set_ri (bool state) { set_bit<SCON_RI >(m_scon, state); }

	void set_pfw(bool state) { set_bit<PCON_PFW>(m_pcon, state); }
	void set_pd (bool state) { set_bit<PCON_PD >(m_pcon, state); }
	void set_idl(bool state) { set_bit<PCON_IDL>(m_pcon, state); }


	void transmit(int state);

	// Memory spaces
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_program;
	memory_access<18, 0, 0, ENDIANNESS_LITTLE>::specific m_data;
	memory_access< 8, 0, 0, ENDIANNESS_LITTLE>::specific m_intd;
	memory_access< 8, 0, 0, ENDIANNESS_LITTLE>::specific m_inti;

	devcb_read8::array<8> m_port_in_cb;
	devcb_write8::array<8> m_port_out_cb;

	// for the debugger
	u8 m_rtemp;

	static const u8 mcs51_cycles[256];
	static const u8 parity_value[256];

	void clear_current_irq();
	virtual offs_t external_ram_iaddr(offs_t offset, offs_t mem_mask);
	virtual void handle_8bit_uart_clock(int source);
	virtual void irqs_complete_and_mask(u8 &ints, u8 int_mask);
	virtual void handle_ta_window();
	virtual bool manage_idle_on_interrupt(u8 ints);
	virtual void handle_irq(int irqline, int state, u32 new_state, u32 tr_state);


	u8 port_default_r(int port);
	void port_default_w(int port, u8 data);

	u8 iram_r(offs_t offset);
	void iram_w(offs_t offset, u8 data);
	void push_pc();
	void pop_pc();
	u8 bit_address_r(u8 offset);
	void bit_address_w(u8 offset, u8 bit);
	void do_add_flags(u8 a, u8 data, u8 c);
	void do_sub_flags(u8 a, u8 data, u8 c);
	void transmit_receive(int source);
	void update_timer_t0(int cycles);
	void update_timer_t1(int cycles);
	virtual void update_timer_t2(int cycles);
	void update_irq_prio();
	void execute_op(u8 op);
	void check_irqs();
	void burn_cycles(int cycles);
	void acall(u8 r);
	void set_reg(u8 r, u8 v);
	u8 r_reg(u8 r);
	void add_a_byte(u8 r);
	void add_a_mem(u8 r);
	void add_a_ir(u8 r);
	void add_a_r(u8 r);
	void addc_a_byte(u8 r);
	void addc_a_mem(u8 r);
	void addc_a_ir(u8 r);
	void addc_a_r(u8 r);
	void ajmp(u8 r);
	void anl_mem_a(u8 r);
	void anl_mem_byte(u8 r);
	void anl_a_byte(u8 r);
	void anl_a_mem(u8 r);
	void anl_a_ir(u8 r);
	void anl_a_r(u8 r);
	void anl_c_bitaddr(u8 r);
	void anl_c_nbitaddr(u8 r);
	void cjne_a_byte(u8 r);
	void cjne_a_mem(u8 r);
	void cjne_ir_byte(u8 r);
	void cjne_r_byte(u8 r);
	void clr_bitaddr(u8 r);
	void clr_c(u8 r);
	void clr_a(u8 r);
	void cpl_bitaddr(u8 r);
	void cpl_c(u8 r);
	void cpl_a(u8 r);
	void da_a(u8 r);
	void dec_a(u8 r);
	void dec_mem(u8 r);
	void dec_ir(u8 r);
	void dec_r(u8 r);
	void div_ab(u8 r);
	void djnz_mem(u8 r);
	void djnz_r(u8 r);
	void inc_a(u8 r);
	void inc_mem(u8 r);
	void inc_ir(u8 r);
	void inc_r(u8 r);
	void inc_dptr(u8 r);
	void jb(u8 r);
	void jbc(u8 r);
	void jc(u8 r);
	void jmp_iadptr(u8 r);
	void jnb(u8 r);
	void jnc(u8 r);
	void jnz(u8 r);
	void jz(u8 r);
	void lcall(u8 r);
	void ljmp(u8 r);
	void mov_a_byte(u8 r);
	void mov_a_mem(u8 r);
	void mov_a_ir(u8 r);
	void mov_a_r(u8 r);
	void mov_mem_byte(u8 r);
	void mov_mem_mem(u8 r);
	void mov_ir_byte(u8 r);
	void mov_r_byte(u8 r);
	void mov_mem_ir(u8 r);
	void mov_mem_r(u8 r);
	void mov_dptr_byte(u8 r);
	void mov_bitaddr_c(u8 r);
	void mov_ir_mem(u8 r);
	void mov_r_mem(u8 r);
	void mov_mem_a(u8 r);
	void mov_ir_a(u8 r);
	void mov_r_a(u8 r);
	void movc_a_iapc(u8 r);
	void mov_c_bitaddr(u8 r);
	void movc_a_iadptr(u8 r);
	void movx_a_idptr(u8 r);
	void movx_a_ir(u8 r);
	void movx_idptr_a(u8 r);
	void movx_ir_a(u8 r);
	void mul_ab(u8 r);
	void nop(u8 r);
	void orl_mem_a(u8 r);
	void orl_mem_byte(u8 r);
	void orl_a_byte(u8 r);
	void orl_a_mem(u8 r);
	void orl_a_ir(u8 r);
	void orl_a_r(u8 r);
	void orl_c_bitaddr(u8 r);
	void orl_c_nbitaddr(u8 r);
	void pop(u8 r);
	void push(u8 r);
	void ret(u8 r);
	void reti(u8 r);
	void rl_a(u8 r);
	void rlc_a(u8 r);
	void rr_a(u8 r);
	void rrc_a(u8 r);
	void setb_c(u8 r);
	void setb_bitaddr(u8 r);
	void sjmp(u8 r);
	void subb_a_byte(u8 r);
	void subb_a_mem(u8 r);
	void subb_a_ir(u8 r);
	void subb_a_r(u8 r);
	void swap_a(u8 r);
	void xch_a_mem(u8 r);
	void xch_a_ir(u8 r);
	void xch_a_r(u8 r);
	void xchd_a_ir(u8 r);
	void xrl_mem_a(u8 r);
	void xrl_mem_byte(u8 r);
	void xrl_a_byte(u8 r);
	void xrl_a_mem(u8 r);
	void xrl_a_ir(u8 r);
	void xrl_a_r(u8 r);
	void illegal(u8 r);
};


// variants with no internal rom and 128 byte internal memory
DECLARE_DEVICE_TYPE(I8031, i8031_device)

// variants 4k internal rom and 128 byte internal memory
DECLARE_DEVICE_TYPE(I8051, i8051_device)
DECLARE_DEVICE_TYPE(I8751, i8751_device)

// variants 8k internal rom and 128 byte internal memory (no 8052 features)
DECLARE_DEVICE_TYPE(AM8753, am8753_device)

DECLARE_DEVICE_TYPE(I8344, i8344_device)
DECLARE_DEVICE_TYPE(I8744, i8744_device)


class i8031_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8031_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8051_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8051_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8751_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8751_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class am8753_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	am8753_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8344_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8344_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class i8744_device : public mcs51_cpu_device
{
public:
	// construction/destruction
	i8744_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};




#endif // MAME_CPU_MCS51_MCS51_H
