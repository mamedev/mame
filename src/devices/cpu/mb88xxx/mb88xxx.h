// license:BSD-3-Clause
// copyright-holders:trwgQ26xxx
/***************************************************************************

	Fujitsu MB8840x / MB8850xH series 4-bit MCU emulator.

	Written by trwgQ26xxx, based on Ernesto Corvi's MB88xx MCU emulator.

	MB8840x series (NMOS): MB88401
	MB8850xH series (CMOS): MB88501H, MB88503H, MB88505H
	Evolved from the MB8840 series.  Key differences vs. the older MB88xx:
		- 12-bit program address space (4 KiB max ROM)
		- 8-bit data address space (up to 256 nibbles RAM)
		- Opcode 0x3D is a two-byte EXT prefix (JPXY/LRXA/AI/LXID/SBA/RBA/
		ICX/RST) instead of the old single-byte JPA instruction
		- CALL/JPL use a 4-bit page field from the opcode (0x60-0x6F / 0x70-0x7F)
		giving a full 12-bit target address

****************************************************************************
              ___________                         ___________
       R4  1 |]          | 42 Vcc          R4  1 |]          | 42 Vcc
       R5  2 |]          | 41 VM           R5  2 |]          | 41 START
       R6  3 |]          | 40 R3           R6  3 |]          | 40 R3
       R7  4 |]          | 39 R2           R7  4 |]          | 39 R2
       R8  5 |]          | 38 R1           R8  5 |]          | 38 R1
       R9  6 |]          | 37 R0           R9  6 |]          | 37 R0
      R10  7 |]          | 36 P3          R10  7 |]          | 36 P3
      R11  8 |]          | 35 P2          R11  8 |]          | 35 P2
      R12  9 |] MB8840x  | 34 P1          R12  9 |] MB8850xH | 34 P1
      R13 10 |]          | 33 P0          R13 10 |]          | 33 P0
      R14 11 |]          | 32 O7          R14 11 |]          | 32 O7
       K0 12 |]          | 31 O6           K0 12 |]          | 31 O6
       K1 13 |]          | 30 O5           K1 13 |]          | 30 O5
       K2 14 |]          | 29 O4           K2 14 |]          | 29 O4
       K3 15 |]          | 28 O3           K3 15 |]          | 28 O3
       EX 16 |]          | 27 O2           EX 16 |]          | 27 O2
        X 17 |]          | 26 O1            X 17 |]          | 26 O1
   _RESET 18 |]          | 25 O0       _RESET 18 |]          | 25 O0
     _IRQ 19 |]          | 24 SO         _IRQ 19 |]          | 24 SO
      _TC 20 |]          | 23 SI          _TC 20 |]          | 23 SI
      Vss 21 |]__________| 22 _SC/_TO     Vss 21 |]__________| 22 _SC/_TO

                MB8840x SERIES                    MB8850xH SERIES
****************************************************************************/

#ifndef MAME_CPU_MB88XXX_MB88XXX_H
#define MAME_CPU_MB88XXX_MB88XXX_H

#pragma once

/***************************************************************************
	REGISTER ENUMERATION
***************************************************************************/

enum
{
	MB88XXX_IRQ_LINE = 0,
	MB88XXX_TC_LINE
};

enum
{
	MB88XXX_PC = 1,
	MB88XXX_PA,
	MB88XXX_FLAGS,
	MB88XXX_SI,
	MB88XXX_A,
	MB88XXX_X,
	MB88XXX_Y,
	MB88XXX_PIO,
	MB88XXX_TH,
	MB88XXX_TL,
	MB88XXX_SB
};

class mb88xxx_cpu_device : public cpu_device
{
public:
	// K (K3-K0): 4-bit parallel input-only port
	auto read_k() { return m_read_k.bind(); }

	// O (O7-O4 = OH, O3-O0 = OL): 8-bit output through optional PLA
	auto write_o() { return m_write_o.bind(); }

	// P (P3-P0): 4-bit output-only port
	auto write_p() { return m_write_p.bind(); }

	// R ports (#0 = R3-R0, #1 = R7-R4, #2 = R11-R8, #3 = R14-R12)
	template <std::size_t Port> auto read_r() { return m_read_r[Port].bind(); }
	template <std::size_t Port> auto write_r() { return m_write_r[Port].bind(); }

	// SI: serial data input
	auto read_si() { return m_read_si.bind(); }
	// SO: serial data output
	auto write_so() { return m_write_so.bind(); }

	// PLA configuration (default: 8-bit pass-through)
	void set_pla_bits(u8 bits) { m_pla_bits = bits; }
	void set_pla_data(u8 *pla) { m_pla_data = pla; }

	void data_c0(address_map &map) ATTR_COLD;		// 192 nibbles (MB88401)
	void data_100(address_map &map) ATTR_COLD;		// 256 nibbles (MB8850xH)
	void program_12bit(address_map &map) ATTR_COLD;	// 4 KiB ROM
	void program_11bit(address_map &map) ATTR_COLD;	// 2 KiB ROM (MB88503H)

protected:
	mb88xxx_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, int data_width, u8 sb_bits);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 3 + 3; } // LRXA(3) + IRQ overhead(3)
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == MB88XXX_IRQ_LINE || inputnum == MB88XXX_TC_LINE; }
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 6 - 1) / 6; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 6); }

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;

	// Registers
	u8	m_PC;		// Program counter, 6 bits (offset within page)
	u8	m_PA;		// Page address, 6 bits -> full addr = (PA<<6)|PC
	u16	m_SP[8];	// 8-deep stack: bits[11:0]=addr, [13]=ST, [14]=ZF, [15]=CF
	u8	m_SI;		// Stack index, 3 bits
	u8	m_A;		// Accumulator, 4 bits
	u8	m_X;		// X register, 4 bits
	u8	m_Y;		// Y register, 4 bits

	// Flags
	u8	m_st;		// Status/condition flag, 1bit
	u8	m_zf;		// Zero flag, 1bit
	u8	m_cf;		// Carry flag, 1bit
	u8	m_vf;		// Timer overflow flag, 1bit
	u8	m_sf;		// Serial buffer full/empty flag, 1bit
	u8	m_if;		// IRQ pin state (logical level, 1=active), 1bit

	// Peripheral control (EN/DIS register)
	u8	m_pio;		// 8-bit: b0=SBI b1=TMR b2=IRQ b3=CLK b4=TO b5=PSC b6=SC b7=TC

	// Timer registers
	u8	m_TH;		// Timer High, 4 bits
	u8	m_TL;		// Timer Low, 4 bits
	u8	m_TP;		// Timer Prescale, 6 bits
	u8	m_ctr;// Last TC pin state (for edge detection)

	// Serial
	u8	m_SB;			// serial buffer, 4 or 8 bits depending on variant
	u16	m_SBcount;		// bits received since last read
	u8	m_sb_bits;		// 4 for most variants, 8 for MB88505H
	emu_timer *m_serial;// timer for serial input/output

	// PLA
	u8 *m_pla_data;	// pointer to PLA data (8-bit pass-through by default)
	u8 m_pla_bits;	// number of bits in PLA output (4 or 8)
	u8 m_o_output;	// latched O-port byte

	// Port callbacks
	devcb_read8 m_read_k;				// 4-bit parallel input-only port
	devcb_write8 m_write_o;				// 8-bit output through optional PLA
	devcb_write8 m_write_p;				// 4-bit output-only port
	devcb_read8::array<4> m_read_r;		// 4-bit input-output ports (R0-R3, R4-R7, R8-R11, R12-R14)
	devcb_write8::array<4> m_write_r;	// 4-bit input-output ports (R0-R3, R4-R7, R8-R11, R12-R14)
	devcb_read_line m_read_si;			// serial data input
	devcb_write_line m_write_so;		// serial data output

	// Interrupt handling
	u8 m_pending_irq;	// pending IRQ state (logical level, 1=active)
	bool m_in_irq;		// whether currently servicing an IRQ (to prevent re-entry)

	// Memory interfaces (12-bit program, 8-bit data for all variants)
	memory_access<12, 0, 0, ENDIANNESS_BIG>::cache m_cache;		// 12-bit program space cache
	memory_access<12, 0, 0, ENDIANNESS_BIG>::specific m_program;// 12-bit program space access
	memory_access< 8, 0, 0, ENDIANNESS_BIG>::specific m_data;	// 8-bit data space access

	int m_icount;		// instruction cycle counter

	// Debugger shadow registers
	u16 m_debugger_pc;	// shadow PC (12 bits)
	u8 m_debugger_flags;// shadow flags (ST/ZF/CF/VF/SF/IF)

	TIMER_CALLBACK_MEMBER(serial_timer);
	void write_pla(u8 index);
	void pio_enable(u8 newpio);
	void increment_timer();
	void burn_cycles(int cycles);
};

/***************************************************************************
	DEVICE TYPE DECLARATIONS
***************************************************************************/

// MB88400 series (NMOS)
class mb88401_cpu_device : public mb88xxx_cpu_device
{
public:
	mb88401_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// MB88500H series (CMOS, high-speed)
class mb88501h_cpu_device : public mb88xxx_cpu_device
{
public:
	mb88501h_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class mb88503h_cpu_device : public mb88xxx_cpu_device  // 2 KiB ROM variant
{
public:
	mb88503h_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class mb88505h_cpu_device : public mb88xxx_cpu_device  // 8-bit serial buffer variant
{
public:
	mb88505h_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(MB88401,  mb88401_cpu_device)
DECLARE_DEVICE_TYPE(MB88501H, mb88501h_cpu_device)
DECLARE_DEVICE_TYPE(MB88503H, mb88503h_cpu_device)
DECLARE_DEVICE_TYPE(MB88505H, mb88505h_cpu_device)

#endif // MAME_CPU_MB88XXX_MB88XXX_H
