// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    cop400.h

    National Semiconductor COPS(COP400 series) emulator.

***************************************************************************/

#ifndef MAME_DEVICES_CPU_COP400_H
#define MAME_DEVICES_CPU_COP400_H

#pragma once

/***************************************************************************
    CONSTANTS
***************************************************************************/

// register access indexes
enum
{
	COP400_PC,
	COP400_SA,
	COP400_SB,
	COP400_SC,
	COP400_A,
	COP400_B,
	COP400_C,
	COP400_G,
	COP400_M,
	COP400_Q,
	COP400_EN,
	COP400_SIO,
	COP400_SKL,
	COP400_T,
	COP400_SKIP
};

// input lines
enum
{
	// COP420
	COP400_IN0 = 0,
	COP400_IN1,
	COP400_IN2,
	COP400_IN3,

	// COP404L
	COP400_MB,
	COP400_DUAL,
	COP400_SEL10,
	COP400_SEL20
};

// CKI bonding options
enum cop400_cki_bond {
	COP400_CKI_DIVISOR_4 = 4,
	COP400_CKI_DIVISOR_8 = 8,
	COP400_CKI_DIVISOR_16 = 16,
	COP400_CKI_DIVISOR_32 = 32
};

// CKO bonding options
enum cop400_cko_bond {
	COP400_CKO_OSCILLATOR_OUTPUT = 0,
	COP400_CKO_RAM_POWER_SUPPLY,
	COP400_CKO_HALT_IO_PORT,
	COP400_CKO_SYNC_INPUT,
	COP400_CKO_GENERAL_PURPOSE_INPUT
};

class cop400_cpu_device : public cpu_device
{
public:
	// L pins: 8-bit bi-directional
	auto read_l() { return m_read_l.bind(); }
	auto write_l() { return m_write_l.bind(); }

	// output state when pins are in tri-state, default 0
	auto read_l_tristate() { return m_read_l_tristate.bind(); }

	// G pins: 4-bit bi-directional
	auto read_g() { return m_read_g.bind(); }
	auto write_g() { return m_write_g.bind(); }

	// D outputs: 4-bit general purpose output
	auto write_d() { return m_write_d.bind(); }

	// IN inputs: 4-bit general purpose input
	auto read_in() { return m_read_in.bind(); }

	// SI/SO lines: serial in/out or counter/gen.purpose
	auto read_si() { return m_read_si.bind(); }
	auto write_so() { return m_write_so.bind(); }

	// SK output line: logic-controlled clock or gen.purpose
	auto write_sk() { return m_write_sk.bind(); }

	// CKI/CKO lines: only CKO input here
	auto read_cko() { return m_read_cko.bind(); }

	void set_config(cop400_cki_bond cki, cop400_cko_bond cko, bool has_microbus)
	{
		set_cki(cki);
		set_cko(cko);
		set_microbus(has_microbus);
	}

	void set_cki(cop400_cki_bond cki) { m_cki = cki; }
	void set_cko(cop400_cko_bond cko) { m_cko = cko; }
	void set_microbus(bool has_microbus) { m_has_microbus = has_microbus; }

	// output pin state accessors
	int so_r() { return m_so_output; }
	int sk_r() { return m_sk_output; }
	uint8_t l_r() { return m_l_output; }

	// microbus
	uint8_t microbus_r();
	void microbus_w(uint8_t data);

	void data_128b(address_map &map) ATTR_COLD;
	void data_32b(address_map &map) ATTR_COLD;
	void data_64b(address_map &map) ATTR_COLD;
	void program_1kb(address_map &map) ATTR_COLD;
	void program_2kb(address_map &map) ATTR_COLD;
	void program_512b(address_map &map) ATTR_COLD;

protected:
	// construction/destruction
	cop400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t program_addr_bits, uint8_t data_addr_bits, uint8_t featuremask, uint8_t g_mask, uint8_t d_mask, uint8_t in_mask, bool has_counter, bool has_inil, address_map_constructor internal_map_program, address_map_constructor internal_map_data);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + m_cki - 1) / m_cki; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * m_cki); }
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 2; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	TIMER_CALLBACK_MEMBER(advance_counter);

	address_space_config m_program_config;
	address_space_config m_data_config;

	// i/o handlers
	devcb_read8 m_read_l;
	devcb_read8 m_read_l_tristate;
	devcb_write8 m_write_l;
	devcb_read8 m_read_g;
	devcb_write8 m_write_g;
	devcb_write8 m_write_d;
	devcb_read8 m_read_in;
	devcb_read_line m_read_si;
	devcb_write_line m_write_so;
	devcb_write_line m_write_sk;
	devcb_read_line m_read_cko;

	enum {
		COP410_FEATURE = 0x01,
		COP420_FEATURE = 0x02,
		COP444L_FEATURE = 0x04,
		COP424C_FEATURE = 0x08
	};

	enum {
		TIMER_SERIAL,
		TIMER_COUNTER,
		TIMER_COUNTER_T,
		TIMER_INIL
	};

	cop400_cki_bond m_cki;
	cop400_cko_bond m_cko;
	bool m_has_microbus;

	bool m_has_counter;
	bool m_has_inil;

	memory_access<11, 0, 0, ENDIANNESS_LITTLE>::cache m_program;
	memory_access< 7, 0, 0, ENDIANNESS_LITTLE>::specific m_data;

	uint8_t m_featuremask;

	// registers
	uint16_t m_pc;             // 9/10/11-bit ROM address program counter
	uint16_t m_prevpc;         // previous value of program counter
	uint8_t m_a;               // 4-bit accumulator
	uint8_t m_b;               // 5/6/7-bit RAM address register
	int m_c;                   // 1-bit carry register
	uint8_t m_en;              // 4-bit enable register
	uint8_t m_g;               // 4-bit general purpose I/O port
	uint8_t m_q;               // 8-bit latch for L port
	uint16_t m_sa, m_sb, m_sc; // subroutine save registers
	uint8_t m_sio;             // 4-bit shift register and counter
	int m_skl;                 // 1-bit latch for SK output

	// counter
	uint8_t m_t;               // 8-bit timer
	int m_skt_latch;           // timer overflow latch

	// input/output ports
	uint8_t m_g_mask;          // G port mask
	uint8_t m_d_mask;          // D port mask
	uint8_t m_in_mask;         // IN port mask
	uint8_t m_il;              // IN latch
	uint8_t m_in[4];           // IN port shift register
	uint8_t m_si;              // serial input
	int m_so_output;           // SO pin output state
	int m_sk_output;           // SK pin output state
	uint8_t m_l_output;        // L pins output state

	// skipping logic
	bool m_skip;               // skip next instruction
	int m_skip_lbi;            // skip until next non-LBI instruction
	bool m_last_skip;          // last value of skip
	bool m_halt;               // halt mode
	bool m_idle;               // idle mode

	// execution logic
	int m_instlen[256];        // instruction length in bytes
	int m_icount;              // instruction counter
	uint8_t m_opcode;          // opcode being executed
	bool m_second_byte;        // second byte of opcode

	// timers
	emu_timer *m_counter_timer;

	typedef void (cop400_cpu_device::*cop400_opcode_func)(uint8_t operand);

	const cop400_opcode_func *m_opcode_map;

	static const cop400_opcode_func COP410_OPCODE_23_MAP[256];
	static const cop400_opcode_func COP410_OPCODE_33_MAP[256];
	static const cop400_opcode_func COP410_OPCODE_MAP[256];
	static const cop400_opcode_func COP420_OPCODE_23_MAP[256];
	static const cop400_opcode_func COP420_OPCODE_33_MAP[256];
	static const cop400_opcode_func COP420_OPCODE_MAP[256];
	static const cop400_opcode_func COP444L_OPCODE_23_MAP[256];
	static const cop400_opcode_func COP444L_OPCODE_33_MAP[256];
	static const cop400_opcode_func COP444L_OPCODE_MAP[256];
	static const cop400_opcode_func COP424C_OPCODE_23_MAP[256];
	static const cop400_opcode_func COP424C_OPCODE_33_MAP[256];
	static const cop400_opcode_func COP424C_OPCODE_MAP[256];

	inline static bool is_control_transfer(uint8_t opcode);

	void serial_tick();
	void counter_tick();
	void inil_tick();

	void PUSH(uint16_t data);
	void POP();
	void WRITE_Q(uint8_t data);
	void WRITE_G(uint8_t data);
	void WRITE_EN(uint8_t data);

	void skip();
	void sk_update();

	uint8_t get_flags() const;
	void set_flags(uint8_t flags);
	uint8_t get_m();
	void set_m(uint8_t m);

	void illegal(uint8_t operand);
	void asc(uint8_t operand);
	void add(uint8_t operand);
	void aisc(uint8_t operand);
	void clra(uint8_t operand);
	void comp(uint8_t operand);
	void nop(uint8_t operand);
	void rc(uint8_t operand);
	void sc(uint8_t operand);
	void xor_(uint8_t operand);
	void adt(uint8_t operand);
	void casc(uint8_t operand);
	void jid(uint8_t operand);
	void jmp(uint8_t operand);
	void jp(uint8_t operand);
	void jsr(uint8_t operand);
	void ret(uint8_t operand);
	void cop420_ret(uint8_t operand);
	void retsk(uint8_t operand);
	void halt(uint8_t operand);
	void it(uint8_t operand);
	void camq(uint8_t operand);
	void ld(uint8_t operand);
	void lqid(uint8_t operand);
	void rmb0(uint8_t operand);
	void rmb1(uint8_t operand);
	void rmb2(uint8_t operand);
	void rmb3(uint8_t operand);
	void smb0(uint8_t operand);
	void smb1(uint8_t operand);
	void smb2(uint8_t operand);
	void smb3(uint8_t operand);
	void stii(uint8_t operand);
	void x(uint8_t operand);
	void xad(uint8_t operand);
	void xds(uint8_t operand);
	void xis(uint8_t operand);
	void cqma(uint8_t operand);
	void ldd(uint8_t operand);
	void camt(uint8_t operand);
	void ctma(uint8_t operand);
	void cab(uint8_t operand);
	void cba(uint8_t operand);
	void lbi(uint8_t operand);
	void lei(uint8_t operand);
	void xabr(uint8_t operand);
	void cop444l_xabr(uint8_t operand);
	void skc(uint8_t operand);
	void ske(uint8_t operand);
	void skgz(uint8_t operand);
	void skgbz0(uint8_t operand);
	void skgbz1(uint8_t operand);
	void skgbz2(uint8_t operand);
	void skgbz3(uint8_t operand);
	void skmbz0(uint8_t operand);
	void skmbz1(uint8_t operand);
	void skmbz2(uint8_t operand);
	void skmbz3(uint8_t operand);
	void skt(uint8_t operand);
	void ing(uint8_t operand);
	void inl(uint8_t operand);
	void obd(uint8_t operand);
	void omg(uint8_t operand);
	void xas(uint8_t operand);
	void inin(uint8_t operand);
	void cop402m_inin(uint8_t operand);
	void inil(uint8_t operand);
	void ogi(uint8_t operand);
	void cop410_op23(uint8_t operand);
	void cop410_op33(uint8_t operand);
	void cop420_op23(uint8_t operand);
	void cop420_op33(uint8_t operand);
	void cop444l_op23(uint8_t operand);
	void cop444l_op33(uint8_t operand);
	void cop424c_op23(uint8_t operand);
	void cop424c_op33(uint8_t operand);
	void skgbz(int bit);
	void skmbz(int bit);
};


/* COP410 family */
// COP401 is a ROMless version of the COP410
class cop401_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop401_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class cop410_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop410_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// COP411 is a 20-pin package version of the COP410, missing D2/D3/G3/CKO
class cop411_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop411_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


/* COP420 family */
// COP402 is a ROMless version of the COP420
class cop402_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop402_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class cop420_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop420_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// COP421 is a 24-pin package version of the COP420, lacking the IN ports
class cop421_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop421_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// COP422 is a 20-pin package version of the COP420, lacking G0/G1, D0/D1, and the IN ports
class cop422_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop422_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


/* COP444 family */
// COP404L is a ROMless version of the COP444, which can emulate a COP410/COP411, COP424/COP425, or a COP444/COP445
class cop404l_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop404l_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class cop444l_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop444l_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// COP445 is a 24-pin package version of the COP444, lacking the IN ports
class cop445l_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop445l_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// COP404C
class cop404c_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop404c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// COP424C is functionally equivalent to COP444C, with only 1K ROM and 64x4 bytes RAM
class cop424c_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop424c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// COP425C is a 24-pin package version of the COP424C, lacking the IN ports
class cop425c_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop425c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// COP426C is a 20-pin package version of the COP424C, with only L0-L7, G2-G3, D2-D3 ports
class cop426c_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop426c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// COP444C
class cop444c_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop444c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// COP445C
class cop445c_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop445c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// COP446C
class cop446c_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop446c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(COP401, cop401_cpu_device)
DECLARE_DEVICE_TYPE(COP410, cop410_cpu_device)
DECLARE_DEVICE_TYPE(COP411, cop411_cpu_device)
DECLARE_DEVICE_TYPE(COP402, cop402_cpu_device)
DECLARE_DEVICE_TYPE(COP420, cop420_cpu_device)
DECLARE_DEVICE_TYPE(COP421, cop421_cpu_device)
DECLARE_DEVICE_TYPE(COP422, cop422_cpu_device)
DECLARE_DEVICE_TYPE(COP404L, cop404l_cpu_device)
DECLARE_DEVICE_TYPE(COP444L, cop444l_cpu_device)
DECLARE_DEVICE_TYPE(COP445L, cop445l_cpu_device)
DECLARE_DEVICE_TYPE(COP404C, cop404c_cpu_device)
DECLARE_DEVICE_TYPE(COP424C, cop424c_cpu_device)
DECLARE_DEVICE_TYPE(COP425C, cop425c_cpu_device)
DECLARE_DEVICE_TYPE(COP426C, cop426c_cpu_device)
DECLARE_DEVICE_TYPE(COP444C, cop444c_cpu_device)
DECLARE_DEVICE_TYPE(COP445C, cop445c_cpu_device)
DECLARE_DEVICE_TYPE(COP446C, cop446c_cpu_device)

#endif  // MAME_DEVICES_CPU_COP400_H
