// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud, David Haywood

#ifndef MAME_CPU_SFR_SFR_H
#define MAME_CPU_SFR_SFR_H

#pragma once

// used for getting / setting current register state in debugger
enum
{
	SFR_STATEREG_PC=1, SFR_STATEREG_SP, SFR_STATEREG_PSW, SFR_STATEREG_ACC, SFR_STATEREG_B, SFR_STATEREG_DPTR0, SFR_STATEREG_DPTR1, SFR_STATEREG_DPH0, SFR_STATEREG_DPL0, SFR_STATEREG_IE, SFR_STATEREG_IP,
	SFR_STATEREG_P0, SFR_STATEREG_P1, SFR_STATEREG_P2, SFR_STATEREG_P3,
	SFR_STATEREG_R0, SFR_STATEREG_R1, SFR_STATEREG_R2, SFR_STATEREG_R3, SFR_STATEREG_R4, SFR_STATEREG_R5, SFR_STATEREG_R6, SFR_STATEREG_R7, SFR_STATEREG_RB,

	SFR_STATEREG_ER0, SFR_STATEREG_ER1, SFR_STATEREG_ER2, SFR_STATEREG_ER3, SFR_ER8,
	SFR_STATEREG_GP0, SFR_STATEREG_GP1, SFR_STATEREG_GP2, SFR_STATEREG_GP3, SFR_STATEREG_GP4, SFR_STATEREG_GP5, SFR_STATEREG_GP6, SFR_STATEREG_GP7,
};

class axc51base_cpu_device : public cpu_device
{
public:
	template <unsigned N> auto port_in_cb() { return m_port_in_cb[N].bind(); }
	template <unsigned N> auto port_out_cb() { return m_port_out_cb[N].bind(); }

	template <unsigned N> auto dac_out_cb() { return m_dac_out_cb[N].bind(); }


	auto spi_in_cb() { return m_spi_in_cb.bind(); }
	auto spi_out_cb() { return m_spi_out_cb.bind(); }
	auto spi_out_dir_cb() { return m_spi_out_dir_cb.bind(); }

	void program_internal(address_map &map) ATTR_COLD;
	void data_internal(address_map &map) ATTR_COLD;
	void io_internal(address_map &map) ATTR_COLD;

protected:
	// construction/destruction
	axc51base_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, int data_width, uint8_t features = 0);
	axc51base_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program_map, address_map_constructor data_map, address_map_constructor io_map, int program_width, int data_width, uint8_t features = 0);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 20; }
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

	int     m_inst_cycles;        /* cycles for the current instruction */
	const uint32_t m_rom_size;    /* size (in bytes) of internal program ROM/EPROM */
	int     m_num_interrupts;     /* number of interrupts supported */
	int     m_recalc_parity;      /* recalculate parity before next instruction */
	uint32_t  m_last_line_state;    /* last state of input lines line */
	int     m_cur_irq_prio;       /* Holds value of the current IRQ Priority Level; -1 if no irq */
	uint8_t   m_irq_active;         /* mask which irq levels are serviced */
	uint8_t   m_irq_prio[8];        /* interrupt priority */

	uint16_t m_spi_dma_addr;


	// JB-related hacks
	uint8_t m_last_op;
	uint8_t m_last_bit;

	int     m_icount;

	/* Internal Ram */
	uint8_t m_sfr_regs[128];
	uint8_t m_xsfr_regs[128];
	required_shared_ptr<uint8_t> m_scratchpad;
	required_shared_ptr<uint8_t> m_mainram;

	uint8_t m_uid[4];

	/* SFR Callbacks */
	void sfr_write(size_t offset, uint8_t data);
	uint8_t sfr_read(size_t offset);

	/* Memory spaces */
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_program;
	memory_access<11, 0, 0, ENDIANNESS_LITTLE>::specific m_data;
	memory_access<17, 0, 0, ENDIANNESS_LITTLE>::specific m_io;

	devcb_read8::array<5> m_port_in_cb;
	devcb_write8::array<5> m_port_out_cb;
	devcb_write8::array<2> m_dac_out_cb;

	devcb_read8 m_spi_in_cb;
	devcb_write8 m_spi_out_cb;
	devcb_write_line m_spi_out_dir_cb;


	// for the debugger
	uint8_t m_rtemp;

	static const uint8_t axc51_cycles[256];

	uint8_t iram_indirect_read(offs_t a);
	void iram_indirect_write(offs_t a, uint8_t d);

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
	void execute_op(uint8_t op);
	void check_irqs();
	void burn_cycles(int cycles);
	uint32_t process_dptr_access();
	uint32_t get_dptr0_with_autoinc(uint8_t auto_inc);
	uint32_t get_dptr1_with_autoinc(uint8_t auto_inc);
	uint8_t xsfr_read(offs_t offset);
	void xsfr_write(offs_t offset, uint8_t data);

	uint8_t read_port(int i);
	void write_port(int i, uint8_t data);

	uint8_t spicon_r();
	uint8_t spibuf_r();
	uint8_t dpcon_r();
	uint8_t uartsta_r();

	void spidmaadr_w(uint8_t data);
	void spidmacnt_w(uint8_t data);

	void spicon_w(uint8_t data);
	void spibuf_w(uint8_t data);
	void spibaud_w(uint8_t data);
	void dpcon_w(uint8_t data);
	void ie2crypt_w(uint8_t data);

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

	void do_ez_flags(uint16_t val);
	void do_ec_ez_flags(uint32_t res);

	void axc51_extended_a5(uint8_t r);
	void extended_a5_0e();
	void extended_a5_0f();
	void extended_a5_d0();
	void extended_a5_d1();

	uint16_t get_erx(int m);
	void set_erx(int n, uint16_t val);
	uint16_t get_dpt(int i);

	/* Internal address in SFR of registers, these map at 0x80, so SFR_P0 is at 0x80 etc. */
	enum
	{
		SFR_P0          = 0x00,
		SFR_SP          = 0x01, // SPL
		SFR_DPL0        = 0x02, // DPL00
		SFR_DPH0        = 0x03, // DPH00
		SFR_DPL1        = 0x04,
		SFR_DPH1        = 0x05,
		SFR_DPCON       = 0x06,
		SFR_PCON        = 0x07, // PCON0
		SFR_SDCON0      = 0x08, // not SFR_TCON
		SFR_SDCON1      = 0x09, // not SFR_TMOD
		SFR_SDCON2      = 0x0a, // not SFR_TL0
		SFR_JPGCON4     = 0x0b, // not SFR_TL1
		SFR_JPGCON3     = 0x0c, // not SFR_TH0
		SFR_JPGCON2     = 0x0d, // not SFR_TH1
		SFR_JPGCON1     = 0x0e,
		SFR_TRAP        = 0x0f,
		SFR_P1          = 0x10,
		SFR_SDBAUD      = 0x11,
		SFR_SDCPTR      = 0x12,
		SFR_SDDCNT      = 0x13,
		SFR_SDDPTR      = 0x14,
		SFR_IE2CRPT     = 0x15, // controls automatic encryption
		SFR_UARTBAUDH   = 0x16,
		SFR_PWKEN       = 0x17,
		SFR_PWKPND      = 0x18, // not SFR_SCON
		SFR_PWKEDGE     = 0x19, // not SFR_SBUF
		SFR_PIE0        = 0x1a,
		SFR_DBASE       = 0x1b,
		SFR_PCON1       = 0x1c,
		SFR_PIE1        = 0x1d,
		SFR_IRTDATA     = 0x1e,
		SFR_IRTCON      = 0x1f,
		SFR_P2          = 0x20,
		SFR_GP0         = 0x21,
		SFR_GP1         = 0x22,
		SFR_GP2         = 0x23,
		SFR_GP3         = 0x24,
		SFR_DACCON      = 0x25,
		SFR_DACLCH      = 0x26,
		SFR_DACRCH      = 0x27,
		SFR_IE          = 0x28, // IE0
		SFR_IE1         = 0x29,
		SFR_KEY0        = 0x2a,
		SFR_KEY1        = 0x2b,
		SFR_TMR3CON     = 0x2c,
		SFR_TMR3CNT     = 0x2d,
		SFR_TMR3PR      = 0x2e,
		SFR_TMR3PSR     = 0x2f,
		SFR_P3          = 0x30,
		SFR_GP4         = 0x31,
		SFR_GP5         = 0x32,
		SFR_GP6         = 0x33,
		SFR_P4          = 0x34,
		SFR_GP7         = 0x35,
		SFR_LCDCON      = 0x36,
		SFR_PLLCON      = 0x37,
		SFR_IP          = 0x38, // IP0
		SFR_IP1         = 0x39,
		SFR_P0DIR       = 0x3a,
		SFR_P1DIR       = 0x3b,
		SFR_P2DIR       = 0x3c,
		SFR_P3DIR       = 0x3d,
		SFR_P4DIR       = 0x3e,
		SFR_LVDCON      = 0x3f,
		SFR_JPGCON0     = 0x40,
		SFR_TMR2CON     = 0x41,
		SFR_JPGCON9     = 0x42,
		SFR_JPGCON5     = 0x43,
		SFR_JPGCON6     = 0x44,
		SFR_JPGCON7     = 0x45,
		SFR_JPGCON8     = 0x46,
		SFR_LCDPR       = 0x47,
		SFR_LCDTCON     = 0x48,
		SFR_USBCON0     = 0x49,
		SFR_USBCON1     = 0x4a,
		SFR_USBCON2     = 0x4b,
		SFR_USBDATA     = 0x4c,
		SFR_USBADR      = 0x4d,
		SFR_ILLEGAL     = 0x4e,
		SFR_MICCON      = 0x4f,
		SFR_PSW         = 0x50,
		SFR_PGCON       = 0x51,
		SFR_ADCCON      = 0x52,
		SFR_PCON2       = 0x53,
		SFR_ADCDATAL    = 0x54,
		SFR_ADCDATAH    = 0x55,
		SFR_SPIDMAADR   = 0x56,
		SFR_SPIDMACNT   = 0x57,
		SFR_SPICON      = 0x58,
		SFR_SPIBUF      = 0x59,
		SFR_SPIBAUD     = 0x5a,
		SFR_CLKCON      = 0x5b,
		SFR_CLKCON1     = 0x5c,
		SFR_USBDPDM     = 0x5d,
		SFR_LFSRPOLY0   = 0x5e,
		SFR_LFSRPOLY1   = 0x5f,
		SFR_ACC         = 0x60,
		SFR_TMR1CON     = 0x61,
		SFR_UID0        = 0x62,
		SFR_UID1        = 0x63,
		SFR_UID2        = 0x64,
		SFR_UID3        = 0x65,
		SFR_ER00        = 0x66,
		SFR_ER01        = 0x67,
		SFR_ER10        = 0x68,
		SFR_ER11        = 0x69,
		SFR_ER20        = 0x6a,
		SFR_ER21        = 0x6b,
		SFR_ER30        = 0x6c,
		SFR_ER31        = 0x6d,
		SFR_ER8         = 0x6e,
		SFR_ILLEGAL2    = 0x6f,
		SFR_B           = 0x70,
		SFR_HUFFBUF     = 0x71,
		SFR_HUFFSFT     = 0x72,
		SFR_HUFFDCL     = 0x73,
		SFR_HUFFDCH     = 0x74,
		SFR_CRC         = 0x75,
		SFR_LFSRFIFO    = 0x76,
		SFR_WDTCON      = 0x77,
		SFR_TMR0CON     = 0x78,
		SFR_TMR0CNT     = 0x79,
		SFR_TMR0PR      = 0x7a,
		SFR_TMR0PSR     = 0x7b,
		SFR_UARTSTA     = 0x7c,
		SFR_UARTCON     = 0x7d,
		SFR_UARTBAUD    = 0x7e,
		SFR_UARTDATA    = 0x7f,
	};

	// XSFR regs map at 0x3000, so XSFR_PUP0 at is 0x3010 etc.
	enum
	{
		XSFR_PUP0 = 0x10,
		XSFR_PUP1 = 0x11,
		XSFR_PUP2 = 0x12,
		XSFR_PUP3 = 0x13,
		XSFR_PUP4 = 0x14,
		XSFR_PDN0 = 0x15,
		XSFR_PDN1 = 0x16,
		XSFR_PDN2 = 0x17,
		XSFR_PDN3 = 0x18,
		XSFR_PDN4 = 0x19,
		XSFR_PHD0 = 0x1a,
		XSFR_PHD1 = 0x1b,
		XSFR_PHD2 = 0x1c,
		XSFR_PHD3 = 0x1d,
		XSFR_PHD4 = 0x1e,

		XSFR_TMR1CNTL = 0x20, // Timer 1 Counter (low)
		XSFR_TMR1CNTH = 0x21, // Timer 1 Counter (high)
		XSFR_TMR1PRL  = 0x22, // Timer 1 Period (low)
		XSFR_TMR1PRH  = 0x23, // Timer 1 Period (high)
		XSFR_TMR1PWML = 0x24, // Timer 1 Duty (low)
		XSFR_TMR1PWMH = 0x25, // Timer 1 Duty (high)

		XSFR_TMR2CNTL = 0x30, // Timer 2 Counter (low)
		XSFR_TMR2CNTH = 0x31, // Timer 2 Counter (high)
		XSFR_TMR2PRL  = 0x32, // Timer 2 Period (low)
		XSFR_TMR2PRH  = 0x33, // Timer 2 Period (high)
		XSFR_TMR2PWML = 0x34, // Timer 2 Duty (low)
		XSFR_TMR2PWMH = 0x35, // Timer 2 Duty (high)

		XSFR_ADCBAUD  = 0x40, // ARADC Baud

		XSFR_USBEP0ADL = 0x50,
		XSFR_USBEP0ADH = 0x51,
		XSFR_USBEP1RXADL = 0x52,
		XSFR_USBEP1RXADH = 0x53,
		XSFR_USBEP1TXADL = 0x54,
		XSFR_USBEP1TXADH = 0x55,
		XSFR_USBEP2RXADL = 0x56,
		XSFR_USBEP2RXADH = 0x57,
		XSFR_USBEP2TXADL = 0x58,
		XSFR_USBEP2TXADH = 0x59,

		XSFR_SFSCON = 0x60,
		XSFR_SFSPID = 0x61,
		XSFR_SFSCNTH = 0x62,
		XSFR_SFSCNTL = 0x63,

		XSFR_DACPTR = 0x70, // DAC DMA Pointer
		XSFR_DACCNT = 0x71, // DAC DMA Counter
	};

	enum
	{
		// always at 8000
		V_RESET = 0x000,    /* power on address */
		// below can be at 4000 or 8000 (although don't make sense for internal ROM at 8000?)
		V_TIMER0     = 0x003,  // IE0.0   IP0.0
		V_TIMER1     = 0x00b,  // IE0.1   IP0.1
		V_TIMER2     = 0x013,  // IE0.2   IP0.2
		V_TIMER3     = 0x01b,  // IE0.3   IP0.3
		V_USB        = 0x023,  // IE0.4   IP0.4
		V_SPI        = 0x02b,  // IE0.5   IP0.5
		V_SDC        = 0x033,  // IE0.6   IP0.6
		V_SOFT       = 0x03b,  // IE2.4   IP0.7  IE2 is the 'encrypt' register
		V_HUFFEMPTY  = 0x043,  // IE1.0   IP1.0
		V_IDCT       = 0x04b,  // IE1.1   IP1.1
		V_YUV2RGB    = 0x053,  // IE1.2   IP1.2
		V_PORT       = 0x05b,  // IE1.3   IP1.3
		V_WDT_LVD    = 0x063,  // IE1.4   IP1.4  Watchdog also needs to be enabled with IE2.5? LVD with LVDCON.5?
		V_IRTCC_UART = 0x06b,  // IE1.5   IP1.5
		V_DAC        = 0x073,  // IE1.6   IP1.6
		V_SFS_INT    = 0x07b,  // IE1.7   IP1.7
	};

	uint16_t get_irq_base();

	TIMER_CALLBACK_MEMBER(timer0_cb);
	TIMER_CALLBACK_MEMBER(dactimer_cb);

	bool m_timer0irq = false;
	bool m_dactimerirq = false;


	emu_timer *m_timer0;
	emu_timer *m_dactimer;
};



DECLARE_DEVICE_TYPE(AX208, ax208_cpu_device)
DECLARE_DEVICE_TYPE(AX208P, ax208p_cpu_device)

class ax208_cpu_device : public axc51base_cpu_device
{
public:
	// construction/destruction
	ax208_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	ax208_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void ax208_internal_program_mem(address_map &map) ATTR_COLD;
};

class ax208p_cpu_device : public ax208_cpu_device
{
public:
	// construction/destruction
	ax208p_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


#endif // MAME_CPU_SFR_SFR_H
