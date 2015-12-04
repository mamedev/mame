// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    cop400.h

    National Semiconductor COPS Emulator.

***************************************************************************/

#pragma once

#ifndef __COP400__
#define __COP400__

// i/o pins

// L pins: 8-bit bi-directional
#define MCFG_COP400_READ_L_CB(_devcb) \
	cop400_cpu_device::set_read_l_callback(*device, DEVCB_##_devcb);
#define MCFG_COP400_WRITE_L_CB(_devcb) \
	cop400_cpu_device::set_write_l_callback(*device, DEVCB_##_devcb);

// G pins: 4-bit bi-directional
#define MCFG_COP400_READ_G_CB(_devcb) \
	cop400_cpu_device::set_read_g_callback(*device, DEVCB_##_devcb);
#define MCFG_COP400_WRITE_G_CB(_devcb) \
	cop400_cpu_device::set_write_g_callback(*device, DEVCB_##_devcb);

// D outputs: 4-bit general purpose output
#define MCFG_COP400_WRITE_D_CB(_devcb) \
	cop400_cpu_device::set_write_d_callback(*device, DEVCB_##_devcb);

// IN inputs: 4-bit general purpose input
#define MCFG_COP400_READ_IN_CB(_devcb) \
	cop400_cpu_device::set_read_in_callback(*device, DEVCB_##_devcb);

// SI/SO lines: serial in/out or counter/gen.purpose
#define MCFG_COP400_READ_SI_CB(_devcb) \
	cop400_cpu_device::set_read_si_callback(*device, DEVCB_##_devcb);
#define MCFG_COP400_WRITE_SO_CB(_devcb) \
	cop400_cpu_device::set_write_so_callback(*device, DEVCB_##_devcb);

// SK output line: logic-controlled clock or gen.purpose
#define MCFG_COP400_WRITE_SK_CB(_devcb) \
	cop400_cpu_device::set_write_sk_callback(*device, DEVCB_##_devcb);

// CKI/CKO lines: only CKO input here
#define MCFG_COP400_READ_CKO_CB(_devcb) \
	cop400_cpu_device::set_read_cko_callback(*device, DEVCB_##_devcb);


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* register access indexes */
enum
{
	COP400_PC,
	COP400_SA,
	COP400_SB,
	COP400_SC,
	COP400_N,
	COP400_A,
	COP400_B,
	COP400_C,
	COP400_G,
	COP400_H,
	COP400_Q,
	COP400_R,
	COP400_EN,
	COP400_SIO,
	COP400_SKL,
	COP400_T,
	COP400_GENPC = STATE_GENPC,
	COP400_GENPCBASE = STATE_GENPCBASE,
	COP400_GENSP = STATE_GENSP
};

/* input lines */
enum
{
	/* COP420 */
	COP400_IN0 = 0,
	COP400_IN1,
	COP400_IN2,
	COP400_IN3,

	/* COP404 */
	COP400_MB,
	COP400_DUAL,
	COP400_SEL10,
	COP400_SEL20
};

/* CKI bonding options */
enum cop400_cki_bond {
	COP400_CKI_DIVISOR_4 = 4,
	COP400_CKI_DIVISOR_8 = 8,
	COP400_CKI_DIVISOR_16 = 16,
	COP400_CKI_DIVISOR_32 = 32
};

/* CKO bonding options */
enum cop400_cko_bond {
	COP400_CKO_OSCILLATOR_OUTPUT = 0,
	COP400_CKO_RAM_POWER_SUPPLY,
	COP400_CKO_HALT_IO_PORT,
	COP400_CKO_SYNC_INPUT,
	COP400_CKO_GENERAL_PURPOSE_INPUT
};

/* microbus bonding options */
enum cop400_microbus {
	COP400_MICROBUS_DISABLED = 0,
	COP400_MICROBUS_ENABLED
};


#define MCFG_COP400_CONFIG(_cki, _cko, _microbus) \
	cop400_cpu_device::set_cki(*device, _cki); \
	cop400_cpu_device::set_cko(*device, _cko); \
	cop400_cpu_device::set_microbus(*device, _microbus);


class cop400_cpu_device : public cpu_device
{
public:
	// construction/destruction
	cop400_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, UINT8 program_addr_bits, UINT8 data_addr_bits, UINT8 featuremask, UINT8 g_mask, UINT8 d_mask, UINT8 in_mask, bool has_counter, bool has_inil, address_map_constructor internal_map_program, address_map_constructor internal_map_data);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_l_callback(device_t &device, _Object object) { return downcast<cop400_cpu_device &>(device).m_read_l.set_callback(object); }
	template<class _Object> static devcb_base &set_write_l_callback(device_t &device, _Object object) { return downcast<cop400_cpu_device &>(device).m_write_l.set_callback(object); }
	template<class _Object> static devcb_base &set_read_g_callback(device_t &device, _Object object) { return downcast<cop400_cpu_device &>(device).m_read_g.set_callback(object); }
	template<class _Object> static devcb_base &set_write_g_callback(device_t &device, _Object object) { return downcast<cop400_cpu_device &>(device).m_write_g.set_callback(object); }
	template<class _Object> static devcb_base &set_write_d_callback(device_t &device, _Object object) { return downcast<cop400_cpu_device &>(device).m_write_d.set_callback(object); }
	template<class _Object> static devcb_base &set_read_in_callback(device_t &device, _Object object) { return downcast<cop400_cpu_device &>(device).m_read_in.set_callback(object); }
	template<class _Object> static devcb_base &set_read_si_callback(device_t &device, _Object object) { return downcast<cop400_cpu_device &>(device).m_read_si.set_callback(object); }
	template<class _Object> static devcb_base &set_write_so_callback(device_t &device, _Object object) { return downcast<cop400_cpu_device &>(device).m_write_so.set_callback(object); }
	template<class _Object> static devcb_base &set_write_sk_callback(device_t &device, _Object object) { return downcast<cop400_cpu_device &>(device).m_write_sk.set_callback(object); }
	template<class _Object> static devcb_base &set_read_cko_callback(device_t &device, _Object object) { return downcast<cop400_cpu_device &>(device).m_read_cko.set_callback(object); }

	static void set_cki(device_t &device, cop400_cki_bond cki) { downcast<cop400_cpu_device &>(device).m_cki = cki; }
	static void set_cko(device_t &device, cop400_cko_bond cko) { downcast<cop400_cpu_device &>(device).m_cko = cko; }
	static void set_microbus(device_t &device, cop400_microbus microbus) { downcast<cop400_cpu_device &>(device).m_microbus = microbus; }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const { return (clocks + m_cki - 1) / m_cki; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const { return (cycles * m_cki); }
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 2; }
	virtual UINT32 execute_input_lines() const { return 0; }
	virtual void execute_run();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const
	{
		return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_DATA) ? &m_data_config : nullptr );
	}

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	address_space_config m_program_config;
	address_space_config m_data_config;

	// i/o handlers
	devcb_read8 m_read_l;
	devcb_write8 m_write_l;
	devcb_read8 m_read_g;
	devcb_write8 m_write_g;
	devcb_write8 m_write_d;
	devcb_read8 m_read_in;
	devcb_read_line m_read_si;
	devcb_write_line m_write_so;
	devcb_write_line m_write_sk;
	devcb_read_line m_read_cko;

	cop400_cki_bond m_cki;
	cop400_cko_bond m_cko;
	cop400_microbus m_microbus;

	bool m_has_counter;
	bool m_has_inil;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;

	UINT8 m_featuremask;

	/* registers */
	UINT16  m_pc;             /* 9/10/11-bit ROM address program counter */
	UINT16  m_prevpc;         /* previous value of program counter */
	UINT8   m_a;              /* 4-bit accumulator */
	UINT8   m_b;              /* 5/6/7-bit RAM address register */
	int     m_c;              /* 1-bit carry register */
	UINT8   m_n;              /* 2-bit stack pointer (COP440 only) */
	UINT8   m_en;             /* 4-bit enable register */
	UINT8   m_g;              /* 4-bit general purpose I/O port */
	UINT8   m_q;              /* 8-bit latch for L port */
	UINT16  m_sa, m_sb, m_sc; /* subroutine save registers (not present in COP440) */
	UINT8   m_sio;            /* 4-bit shift register and counter */
	int     m_skl;            /* 1-bit latch for SK output */
	UINT8   m_h;              /* 4-bit general purpose I/O port (COP440 only) */
	UINT8   m_r;              /* 8-bit general purpose I/O port (COP440 only) */
	UINT8   m_flags;          // used for I/O only

	/* counter */
	UINT8   m_t;              /* 8-bit timer */
	int     m_skt_latch;      /* timer overflow latch */

	/* input/output ports */
	UINT8   m_g_mask;         /* G port mask */
	UINT8   m_d_mask;         /* D port mask */
	UINT8   m_in_mask;        /* IN port mask */
	UINT8   m_il;             /* IN latch */
	UINT8   m_in[4];          /* IN port shift register */
	UINT8   m_si;             /* serial input */

	/* skipping logic */
	int m_skip;               /* skip next instruction */
	int m_skip_lbi;           /* skip until next non-LBI instruction */
	int m_last_skip;          /* last value of skip */
	int m_halt;               /* halt mode */
	int m_idle;               /* idle mode */

	/* microbus */
	int m_microbus_int;       /* microbus interrupt */

	/* execution logic */
	int m_InstLen[256];       /* instruction length in bytes */
	int m_icount;             /* instruction counter */

	/* timers */
	emu_timer *m_serial_timer;
	emu_timer *m_counter_timer;
	emu_timer *m_inil_timer;
	emu_timer *m_microbus_timer;

	typedef void ( cop400_cpu_device::*cop400_opcode_func ) (UINT8 opcode);

	/* The opcode table now is a combination of cycle counts and function pointers */
	struct cop400_opcode_map {
		UINT32 cycles;
		cop400_opcode_func function;
	};

	const cop400_opcode_map *m_opcode_map;

	static const cop400_opcode_map COP410_OPCODE_23_MAP[256];
	static const cop400_opcode_map COP410_OPCODE_33_MAP[256];
	static const cop400_opcode_map COP410_OPCODE_MAP[256];
	static const cop400_opcode_map COP420_OPCODE_23_MAP[256];
	static const cop400_opcode_map COP420_OPCODE_33_MAP[256];
	static const cop400_opcode_map COP420_OPCODE_MAP[256];
	static const cop400_opcode_map COP444_OPCODE_23_MAP[256];
	static const cop400_opcode_map COP444_OPCODE_33_MAP[256];
	static const cop400_opcode_map COP444_OPCODE_MAP[256];

	void serial_tick();
	void counter_tick();
	void inil_tick();
	void microbus_tick();

	void PUSH(UINT16 data);
	void POP();
	void WRITE_Q(UINT8 data);
	void WRITE_G(UINT8 data);

	void illegal(UINT8 opcode);
	void asc(UINT8 opcode);
	void add(UINT8 opcode);
	void aisc(UINT8 opcode);
	void clra(UINT8 opcode);
	void comp(UINT8 opcode);
	void nop(UINT8 opcode);
	void rc(UINT8 opcode);
	void sc(UINT8 opcode);
	void xor_(UINT8 opcode);
	void adt(UINT8 opcode);
	void casc(UINT8 opcode);
	void jid(UINT8 opcode);
	void jmp(UINT8 opcode);
	void jp(UINT8 opcode);
	void jsr(UINT8 opcode);
	void ret(UINT8 opcode);
	void cop420_ret(UINT8 opcode);
	void retsk(UINT8 opcode);
	void halt(UINT8 opcode);
	void it(UINT8 opcode);
	void camq(UINT8 opcode);
	void ld(UINT8 opcode);
	void lqid(UINT8 opcode);
	void rmb0(UINT8 opcode);
	void rmb1(UINT8 opcode);
	void rmb2(UINT8 opcode);
	void rmb3(UINT8 opcode);
	void smb0(UINT8 opcode);
	void smb1(UINT8 opcode);
	void smb2(UINT8 opcode);
	void smb3(UINT8 opcode);
	void stii(UINT8 opcode);
	void x(UINT8 opcode);
	void xad(UINT8 opcode);
	void xds(UINT8 opcode);
	void xis(UINT8 opcode);
	void cqma(UINT8 opcode);
	void ldd(UINT8 opcode);
	void camt(UINT8 opcode);
	void ctma(UINT8 opcode);
	void cab(UINT8 opcode);
	void cba(UINT8 opcode);
	void lbi(UINT8 opcode);
	void lei(UINT8 opcode);
	void xabr(UINT8 opcode);
	void cop444_xabr(UINT8 opcode);
	void skc(UINT8 opcode);
	void ske(UINT8 opcode);
	void skgz(UINT8 opcode);
	void skgbz0(UINT8 opcode);
	void skgbz1(UINT8 opcode);
	void skgbz2(UINT8 opcode);
	void skgbz3(UINT8 opcode);
	void skmbz0(UINT8 opcode);
	void skmbz1(UINT8 opcode);
	void skmbz2(UINT8 opcode);
	void skmbz3(UINT8 opcode);
	void skt(UINT8 opcode);
	void ing(UINT8 opcode);
	void inl(UINT8 opcode);
	void obd(UINT8 opcode);
	void omg(UINT8 opcode);
	void xas(UINT8 opcode);
	void inin(UINT8 opcode);
	void cop402m_inin(UINT8 opcode);
	void inil(UINT8 opcode);
	void ogi(UINT8 opcode);
	void cop410_op23(UINT8 opcode);
	void cop410_op33(UINT8 opcode);
	void cop420_op23(UINT8 opcode);
	void cop420_op33(UINT8 opcode);
	void cop444_op23(UINT8 opcode);
	void cop444_op33(UINT8 opcode);
	void skgbz(int bit);
	void skmbz(int bit);

};


/* COP410 family */
// COP401 is a ROMless version of the COP410
class cop401_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop401_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class cop410_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop410_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// COP411 is a 20-pin package version of the COP410, missing D2/D3/G3/CKO
class cop411_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop411_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


/* COP420 family */
// COP402 is a ROMless version of the COP420
class cop402_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop402_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class cop420_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop420_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// COP421 is a 24-pin package version of the COP420, lacking the IN ports
class cop421_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop421_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// COP422 is a 20-pin package version of the COP420, lacking G0/G1, D0/D1, and the IN ports
class cop422_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop422_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


/* COP444 family */
// COP404 is a ROMless version of the COP444, which can emulate a COP410C/COP411C, COP424C/COP425C, or a COP444C/COP445C
class cop404_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop404_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// COP424 is functionally equivalent to COP444, with only 1K ROM and 64x4 bytes RAM
class cop424_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop424_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// COP425 is a 24-pin package version of the COP424, lacking the IN ports
class cop425_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop425_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// COP426 is a 20-pin package version of the COP424, with only L0-L7, G2-G3, D2-D3 ports
class cop426_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop426_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class cop444_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop444_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// COP445 is a 24-pin package version of the COP444, lacking the IN ports
class cop445_cpu_device : public cop400_cpu_device
{
public:
	// construction/destruction
	cop445_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type COP401;
extern const device_type COP410;
extern const device_type COP411;
extern const device_type COP402;
extern const device_type COP420;
extern const device_type COP421;
extern const device_type COP422;
extern const device_type COP404;
extern const device_type COP424;
extern const device_type COP425;
extern const device_type COP426;
extern const device_type COP444;
extern const device_type COP445;


#endif  /* __COP400__ */
