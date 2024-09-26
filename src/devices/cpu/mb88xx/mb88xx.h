// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    mb88xx.h
    Core implementation for the portable Fujitsu MB88xx series MCU emulator.

    Written by Ernesto Corvi

****************************************************************************
              ___________                         ___________
       EX  1 |]          | 42 Vcc          EX  1 |]          | 28 Vcc
        X  2 |]          | 41 K3            X  2 |]          | 27 K3
   _RESET  3 |]          | 40 K2       _RESET  3 |]          | 26 K2
     _IRQ  4 |]          | 39 K1           O0  4 |]          | 25 K1
       SO  5 |]          | 38 K0           O1  5 |]          | 24 K0
       SI  6 |]          | 37 R15          O2  6 |]          | 23 R10/_IRQ
  _SC/_TO  7 |]          | 36 R14          O3  7 |]  MB8842  | 22 R9/_TC
      _TC  8 |]          | 35 R13          O4  8 |]  MB8844  | 21 R8
       P0  9 |]          | 34 R12          O5  9 |]          | 20 R7
       P1 10 |]  MB8841  | 33 R11          O6 10 |]          | 19 R6
       P2 11 |]  MB8843  | 32 R10          O7 11 |]          | 18 R5
       P3 12 |]          | 31 R9           R0 12 |]          | 17 R4
       O0 13 |]          | 30 R8           R1 13 |]          | 16 R3
       O1 14 |]          | 29 R7          Vss 14 |]__________| 15 R2
       O2 15 |]          | 28 R6
       O3 16 |]          | 27 R5
       O4 17 |]          | 26 R4
       O5 18 |]          | 25 R3
       O6 19 |]          | 24 R2
       O7 20 |]          | 23 R1
      Vss 21 |]__________| 22 R0
                                 ___   ___
                          R6  1 |   \_/   | 16 Vcc
                          R7  2 |         | 15 _RESET
                          R8  3 |         | 14 R5
                          R9  4 | MB88201 | 13 R4
                   R10/START  5 | MB88202 | 12 R3
                       R11/X  6 |         | 11 R2
                          EX  7 |         | 10 R1
                         Vss  8 |_________| 9  R0

***************************************************************************/

#ifndef MAME_CPU_MB88XX_MB88XX_H
#define MAME_CPU_MB88XX_MB88XX_H

#pragma once


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	MB88_PC = 1,
	MB88_PA,
	MB88_FLAGS,
	MB88_SI,
	MB88_A,
	MB88_X,
	MB88_Y,
	MB88_PIO,
	MB88_TH,
	MB88_TL,
	MB88_SB
};

#define MB88_IRQ_LINE       0


class mb88_cpu_device : public cpu_device
{
public:
	// configuration helpers

	// K (K3-K0): input-only port
	auto read_k() { return m_read_k.bind(); }

	// O (O7-O4 = OH, O3-O0 = OL): output through PLA
	auto write_o() { return m_write_o.bind(); }

	// P (P3-P0): output-only port
	auto write_p() { return m_write_p.bind(); }

	// R0 (R3-R0): input/output port
	template <std::size_t Port> auto read_r() { return m_read_r[Port].bind(); }
	template <std::size_t Port> auto write_r() { return m_write_r[Port].bind(); }

	// SI: serial input
	auto read_si() { return m_read_si.bind(); }

	// SO: serial output
	auto write_so() { return m_write_so.bind(); }

	void set_pla(u8 *pla) { m_PLA = pla; }

	void clock_w(int state);

	void data_4bit(address_map &map) ATTR_COLD;
	void data_5bit(address_map &map) ATTR_COLD;
	void data_6bit(address_map &map) ATTR_COLD;
	void data_7bit(address_map &map) ATTR_COLD;
	void program_10bit(address_map &map) ATTR_COLD;
	void program_11bit(address_map &map) ATTR_COLD;
	void program_9bit(address_map &map) ATTR_COLD;

protected:
	mb88_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, int data_width);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 3; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
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

	u8   m_PC;     // Program Counter: 6 bits
	u8   m_PA;     // Page Address: 4 bits
	u16  m_SP[4];  // Stack is 4*10 bit addresses deep, but we also use 3 top bits per address to store flags during irq
	u8   m_SI;     // Stack index: 2 bits
	u8   m_A;      // Accumulator: 4 bits
	u8   m_X;      // Index X: 4 bits
	u8   m_Y;      // Index Y: 4 bits
	u8   m_st;     // State flag: 1 bit
	u8   m_zf;     // Zero flag: 1 bit
	u8   m_cf;     // Carry flag: 1 bit
	u8   m_vf;     // Timer overflow flag: 1 bit
	u8   m_sf;     // Serial Full/Empty flag: 1 bit
	u8   m_if;     // Interrupt flag: 1 bit

	// Peripheral Control
	u8   m_pio;    // Peripheral enable bits: 8 bits

	// Timer registers
	u8   m_TH;     // Timer High: 4 bits
	u8   m_TL;     // Timer Low: 4 bits
	u8   m_TP;     // Timer Prescale: 6 bits?
	u8   m_ctr;    // current external counter value

	// Serial registers
	u8   m_SB;     // Serial buffer: 4 bits
	u16  m_SBcount;// number of bits received
	emu_timer *m_serial;

	// PLA configuration and port callbacks
	u8 *m_PLA;
	devcb_read8 m_read_k;
	devcb_write8 m_write_o;
	devcb_write8 m_write_p;
	devcb_read8::array<4> m_read_r;
	devcb_write8::array<4> m_write_r;
	devcb_read_line m_read_si;
	devcb_write_line m_write_so;

	// IRQ handling
	u8 m_pending_interrupt;
	bool m_in_irq;

	memory_access<11, 0, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<11, 0, 0, ENDIANNESS_BIG>::specific m_program;
	memory_access< 7, 0, 0, ENDIANNESS_BIG>::specific m_data;

	int m_icount;

	// For the debugger
	u16 m_debugger_pc;
	u8 m_debugger_flags;

	TIMER_CALLBACK_MEMBER(serial_timer);
	int pla(int inA, int inB);
	void update_pio_enable(u8 newpio);
	void increment_timer();
	void update_pio(int cycles);
};


class mb88201_cpu_device : public mb88_cpu_device
{
public:
	mb88201_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class mb88202_cpu_device : public mb88_cpu_device
{
public:
	mb88202_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class mb8841_cpu_device : public mb88_cpu_device
{
public:
	mb8841_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class mb8842_cpu_device : public mb88_cpu_device
{
public:
	mb8842_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class mb8843_cpu_device : public mb88_cpu_device
{
public:
	mb8843_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class mb8844_cpu_device : public mb88_cpu_device
{
public:
	mb8844_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(MB88201, mb88201_cpu_device)
DECLARE_DEVICE_TYPE(MB88202, mb88202_cpu_device)
DECLARE_DEVICE_TYPE(MB8841,  mb8841_cpu_device)
DECLARE_DEVICE_TYPE(MB8842,  mb8842_cpu_device)
DECLARE_DEVICE_TYPE(MB8843,  mb8843_cpu_device)
DECLARE_DEVICE_TYPE(MB8844,  mb8844_cpu_device)

#endif // MAME_CPU_MB88XX_MB88XX_H
