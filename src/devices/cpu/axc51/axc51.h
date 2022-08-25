// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

#ifndef MAME_CPU_AXC51_AXC51_H
#define MAME_CPU_AXC51_AXC51_H

#pragma once


enum
{
	AXC51_PC=1, AXC51_SP, AXC51_PSW, AXC51_ACC, AXC51_B, AXC51_DPTR, AXC51_DPH, AXC51_DPL, AXC51_IE, AXC51_IP,
	AXC51_P0, AXC51_P1, AXC51_P2, AXC51_P3,
	AXC51_R0, AXC51_R1, AXC51_R2, AXC51_R3, AXC51_R4, AXC51_R5, AXC51_R6, AXC51_R7, AXC51_RB,
};

enum
{
	AXC51_INT0_LINE = 0,    /* P3.2: External Interrupt 0 */
	AXC51_INT1_LINE,        /* P3.3: External Interrupt 1 */
	AXC51_RX_LINE,          /* P3.0: Serial Port Receive Line */
	AXC51_T0_LINE,          /* P3,4: Timer 0 External Input */
	AXC51_T1_LINE,          /* P3.5: Timer 1 External Input */
	AXC51_T2_LINE,          /* P1.0: Timer 2 External Input */
	AXC51_T2EX_LINE,        /* P1.1: Timer 2 Capture Reload Trigger */
};


class axc51base_cpu_device : public cpu_device
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
	axc51base_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, int data_width, uint8_t features = 0);
	axc51base_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program_map, address_map_constructor data_map, int program_width, int data_width, uint8_t features = 0);

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

	uint8_t   m_forced_inputs[4];   /* allow read even if configured as output */

	// JB-related hacks
	uint8_t m_last_op;
	uint8_t m_last_bit;

	int     m_icount;

	struct axc51_uart
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

	// for the debugger
	uint8_t m_rtemp;

	static const uint8_t axc51_cycles[256];

	uint8_t iram_iread(offs_t a);
	void iram_iwrite(offs_t a, uint8_t d);
	void clear_current_irq();
	uint8_t r_acc();
	uint8_t r_psw();
	virtual offs_t external_ram_iaddr(offs_t offset, offs_t mem_mask);
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


	/* Internal address in SFR of registers */
	enum
	{
		ADDR_P0          = 0x80,
		ADDR_SP          = 0x81, // SPL
		ADDR_DPL         = 0x82, // DPL0
		ADDR_DPH         = 0x83, // DPH0
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
	};

	enum
	{
		V_RESET = 0x000,    /* power on address */
		V_IE0   = 0x003,    /* External Interrupt 0 */
	};

};



DECLARE_DEVICE_TYPE(AX208, ax208_cpu_device)
DECLARE_DEVICE_TYPE(AX208P, ax208p_cpu_device)

class ax208_cpu_device : public axc51base_cpu_device
{
public:
	// construction/destruction
	ax208_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_spi_ptr(uint8_t* ptr, size_t size) { m_spiptr = ptr; m_spisize = size; }

protected:
	ax208_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_reset() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	void ax208_internal_program_mem(address_map &map);

	virtual void sfr_write(size_t offset, uint8_t data) override;
	virtual uint8_t sfr_read(size_t offset) override;

	uint8_t spicon_r();
	uint8_t spibuf_r();
	uint8_t dpcon_r();

	void spidmaadr_w(uint8_t data);
	void spidmacnt_w(uint8_t data);

	void spicon_w(uint8_t data);
	void spibuf_w(uint8_t data);
	void spibaud_w(uint8_t data);
	void dpcon_w(uint8_t data);

	uint8_t* m_spiptr;
	size_t m_spisize;

	uint32_t m_spiaddr;

	virtual offs_t external_ram_iaddr(offs_t offset, offs_t mem_mask) override;
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
