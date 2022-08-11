// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud, David Haywood

#ifndef MAME_CPU_AXC51_AXC51_H
#define MAME_CPU_AXC51_AXC51_H

#pragma once


enum
{
	AXC51_PC=1, AXC51_SP, AXC51_PSW, AXC51_ACC, AXC51_B, AXC51_DPTR0, AXC51_DPTR1, AXC51_DPH0, AXC51_DPL0, AXC51_IE, AXC51_IP,
	AXC51_P0, AXC51_P1, AXC51_P2, AXC51_P3,
	AXC51_R0, AXC51_R1, AXC51_R2, AXC51_R3, AXC51_R4, AXC51_R5, AXC51_R6, AXC51_R7, AXC51_RB,

	AXC51_ER0, AXC51_ER1, AXC51_ER2, AXC51_ER3, AXC51_REG_ER8,
	AXC51_REG_GP0, AXC51_REG_GP1, AXC51_REG_GP2, AXC51_REG_GP3, AXC51_REG_GP4, AXC51_REG_GP5, AXC51_REG_GP6, AXC51_REG_GP7, 
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

	void program_internal(address_map &map);
	void data_internal(address_map &map);
	void io_internal(address_map &map);

protected:
	// construction/destruction
	axc51base_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, int data_width, uint8_t features = 0);
	axc51base_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program_map, address_map_constructor data_map, address_map_constructor io_map, int program_width, int data_width, uint8_t features = 0);

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

	/* Internal address in SFR of registers */
	enum
	{
		ADDR_P0          = 0x80,
		ADDR_SP          = 0x81, // SPL
		ADDR_DPL0        = 0x82, // DPL00
		ADDR_DPH0        = 0x83, // DPH00
		AXC51_DPL1       = 0x84,
		AXC51_DPH1       = 0x85,
		AXC51_DPCON      = 0x86,
		ADDR_PCON        = 0x87, // PCON0
		AXC51_SDCON0     = 0x88, // not ADDR_TCON   = 0x88,
		AXC51_SDCON1     = 0x89, // not ADDR_TMOD   = 0x89,
		AXC51_SDCON2     = 0x8a, // not ADDR_TL0    = 0x8a,
		AXC51_JPGCON4    = 0x8b, // not ADDR_TL1    = 0x8b,
		AXC51_JPGCON3    = 0x8c, // not ADDR_TH0    = 0x8c,
		AXC51_JPGCON2    = 0x8d, // not ADDR_TH1    = 0x8d,
		AXC51_JPGCON1    = 0x8e,
		AXC51_TRAP       = 0x8f,
		ADDR_P1          = 0x90,
		AXC51_SDBAUD     = 0x91,
		AXC51_SDCPTR     = 0x92,
		AXC51_SDDCNT     = 0x93,
		AXC51_SDDPTR     = 0x94,
		AXC51_IE2CRPT    = 0x95, // controls automatic encryption
		AXC51_UARTBAUDH  = 0x96,
		AXC51_PWKEN      = 0x97,
		AXC51_PWKPND     = 0x98, // not ADDR_SCON   = 0x98,
		AXC51_PWKEDGE    = 0x99, // not ADDR_SBUF   = 0x99,
		AXC51_PIE0       = 0x9a,
		AXC51_DBASE      = 0x9b,
		AXC51_PCON1      = 0x9c,
		AXC51_PIE1       = 0x9d,
		AXC51_IRTDATA    = 0x9e,
		AXC51_IRTCON     = 0x9f,
		ADDR_P2          = 0xa0,
		AXC51_GP0        = 0xa1,
		AXC51_GP1        = 0xa2,
		AXC51_GP2        = 0xa3,
		AXC51_GP3        = 0xa4,
		AXC51_DACCON     = 0xa5,
		AXC51_DACLCH     = 0xa6,
		AXC51_DACRCH     = 0xa7,
		ADDR_IE          = 0xa8, // IE0
		AXC51_IE1        = 0xa9,
		AXC51_KEY0       = 0xaa,
		AXC51_KEY1       = 0xab,
		AXC51_TMR3CON    = 0xac,
		AXC51_TMR3CNT    = 0xad,
		AXC51_TMR3PR     = 0xae,
		AXC51_TMR3PSR    = 0xaf,
		ADDR_P3          = 0xb0,
		AXC51_GP4        = 0xb1,
		AXC51_GP5        = 0xb2,
		AXC51_GP6        = 0xb3,
		AXC51_P4         = 0xb4,
		AXC51_GP7        = 0xb5,
		AXC51_LCDCON     = 0xb6,
		AXC51_PLLCON     = 0xb7,
		ADDR_IP          = 0xb8, // IP0
		AXC51_IP1        = 0xb9,
		AXC51_P0DIR      = 0xba,
		AXC51_P1DIR      = 0xbb,
		AXC51_P2DIR      = 0xbc,
		AXC51_P3DIR      = 0xbd,
		AXC51_P4DIR      = 0xbe,
		AXC51_LVDCON     = 0xbf,
		AXC51_JPGCON0    = 0xc0,
		AXC51_TMR2CON    = 0xc1,
		AXC51_JPGCON9    = 0xc2,
		AXC51_JPGCON5    = 0xc3,
		AXC51_JPGCON6    = 0xc4,
		AXC51_JPGCON7    = 0xc5,
		AXC51_JPGCON8    = 0xc6,
		AXC51_LCDPR      = 0xc7,
		AXC51_LCDTCON    = 0xc8,
		AXC51_USBCON0    = 0xc9,
		AXC51_USBCON1    = 0xca,
		AXC51_USBCON2    = 0xcb,
		AXC51_USBDATA    = 0xcc,
		AXC51_USBADR     = 0xcd,
		AXC51_ILLEGAL    = 0xce,
		AXC51_MICCON     = 0xcf,
		ADDR_PSW         = 0xd0,
		AXC51_PGCON      = 0xd1,
		AXC51_ADCCON     = 0xd2,
		AXC51_PCON2      = 0xd3,
		AXC51_ADCDATAL   = 0xd4,
		AXC51_ADCDATAH   = 0xd5,
		AXC51_SPIDMAADR  = 0xd6,
		AXC51_SPIDMACNT  = 0xd7,
		AXC51_SPICON     = 0xd8,
		AXC51_SPIBUF     = 0xd9,
		AXC51_SPIBAUD    = 0xda,
		AXC51_CLKCON     = 0xdb,
		AXC51_CLKCON1    = 0xdc,
		AXC51_USBDPDM    = 0xdd,
		AXC51_LFSRPOLY0  = 0xde,
		AXC51_LFSRPOLY1  = 0xdf,
		ADDR_ACC         = 0xe0,
		AXC51_TMR1CON    = 0xe1,
		AXC51_UID0       = 0xe2,
		AXC51_UID1       = 0xe3,
		AXC51_UID2       = 0xe4,
		AXC51_UID3       = 0xe5,
		AXC51_ER00       = 0xe6,
		AXC51_ER01       = 0xe7,
		AXC51_ER10       = 0xe8,
		AXC51_ER11       = 0xe9,
		AXC51_ER20       = 0xea,
		AXC51_ER21       = 0xeb,
		AXC51_ER30       = 0xec,
		AXC51_ER31       = 0xed,
		AXC51_ER8        = 0xee,
		AXC51_ILLEGAL2   = 0xef,
		ADDR_B           = 0xf0,
		AXC51_HUFFBUF    = 0xf1,
		AXC51_HUFFSFT    = 0xf2,
		AXC51_HUFFDCL    = 0xf3,
		AXC51_HUFFDCH    = 0xf4,
		AXC51_CRC        = 0xf5,
		AXC51_LFSRFIFO   = 0xf6,
		AXC51_WDTCON     = 0xf7,
		AXC51_TMR0CON    = 0xf8,
		AXC51_TMR0CNT    = 0xf9,
		AXC51_TMR0PR     = 0xfa,
		AXC51_TMR0PSR    = 0xfb,
		AXC51_UARTSTA    = 0xfc,
		AXC51_UARTCON    = 0xfd,
		AXC51_UARTBAUD   = 0xfe,
		AXC51_UARTDATA   = 0xff,

		// XSFR
		AXC51_PUP0 = 0x3010,
		AXC51_PUP1 = 0x3011,
		AXC51_PUP2 = 0x3012,
		AXC51_PUP3 = 0x3013,
		AXC51_PUP4 = 0x3014,
		AXC51_PDN0 = 0x3015,
		AXC51_PDN1 = 0x3016,
		AXC51_PDN2 = 0x3017,
		AXC51_PDN3 = 0x3018,
		AXC51_PDN4 = 0x3019,
		AXC51_PHD0 = 0x301a,
		AXC51_PHD1 = 0x301b,
		AXC51_PHD2 = 0x301c,
		AXC51_PHD3 = 0x301d,
		AXC51_PHD4 = 0x301e,

		AXC51_TMR1CNTL = 0x3020, // Timer 1 Counter (low)
		AXC51_TMR1CNTH = 0x3021, // Timer 1 Counter (high)
		AXC51_TMR1PRL  = 0x3022, // Timer 1 Period (low)
		AXC51_TMR1PRH  = 0x3023, // Timer 1 Period (high)
		AXC51_TMR1PWML = 0x3024, // Timer 1 Duty (low)
		AXC51_TMR1PWMH = 0x3025, // Timer 1 Duty (high)

		AXC51_TMR2CNTL = 0x3030, // Timer 2 Counter (low)
		AXC51_TMR2CNTH = 0x3031, // Timer 2 Counter (high)
		AXC51_TMR2PRL  = 0x3032, // Timer 2 Period (low)
		AXC51_TMR2PRH  = 0x3033, // Timer 2 Period (high)
		AXC51_TMR2PWML = 0x3034, // Timer 2 Duty (low)
		AXC51_TMR2PWMH = 0x3035, // Timer 2 Duty (high)

		AXC51_ADCBAUD  = 0x3040, // ARADC Baud

		AXC51_USBEP0ADL = 0x3050,
		AXC51_USBEP0ADH = 0x3051,
		AXC51_USBEP1RXADL = 0x3052,
		AXC51_USBEP1RXADH = 0x3053,
		AXC51_USBEP1TXADL = 0x3054,
		AXC51_USBEP1TXADH = 0x3055,
		AXC51_USBEP2RXADL = 0x3056,
		AXC51_USBEP2RXADH = 0x3057,
		AXC51_USBEP2TXADL = 0x3058,
		AXC51_USBEP2TXADH = 0x3059,

		AXC51_SFSCON = 0x3060,
		AXC51_SFSPID = 0x3061,
		AXC51_SFSCNTH = 0x3062,
		AXC51_SFSCNTL = 0x3063,

		AXC51_DACPTR = 0x3070, // DAC DMA Pointer
		AXC51_DACCNT = 0x3071, // DAC DMA Counter
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

	virtual void device_reset() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	void ax208_internal_program_mem(address_map &map);
};

class ax208p_cpu_device : public ax208_cpu_device
{
public:
	// construction/destruction
	ax208p_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
};


#endif // MAME_CPU_AXC51_AXC51_H
