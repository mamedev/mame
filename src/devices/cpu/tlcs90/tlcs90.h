// license:BSD-3-Clause
// copyright-holders:Luca Elia
#pragma once

#ifndef __TLCS90_H__
#define __TLCS90_H__


#define T90_IOBASE  0xffc0

enum e_ir
{
	T90_P0=T90_IOBASE,  T90_P1,     T90_P01CR_IRFL, T90_IRFH,   T90_P2,     T90_P2CR,   T90_P3,     T90_P3CR,
	T90_P4,             T90_P4CR,   T90_P5,         T90_SMMOD,  T90_P6,     T90_P7,     T90_P67CR,  T90_SMCR,
	T90_P8,             T90_P8CR,   T90_WDMOD,      T90_WDCR,   T90_TREG0,  T90_TREG1,  T90_TREG2,  T90_TREG3,
	T90_TCLK,           T90_TFFCR,  T90_TMOD,       T90_TRUN,   T90_CAP1L,  T90_CAP1H,  T90_CAP2L,  T90_CAL2H,
	T90_TREG4L,         T90_TREG4H, T90_TREG5L,     T90_TREG5H, T90_T4MOD,  T90_T4FFCR, T90_INTEL,  T90_INTEH,
	T90_DMAEH,          T90_SCMOD,  T90_SCCR,       T90_SCBUF,  T90_BX,     T90_BY,     T90_ADREG,  T90_ADMOD
};

enum tlcs90_e_irq {    INTSWI = 0, INTNMI, INTWD,  INT0,   INTT0,  INTT1,  INTT2,  INTT3,  INTT4,  INT1,   INTT5,  INT2,   INTRX,  INTTX,  INTMAX  };
DECLARE_ENUM_OPERATORS(tlcs90_e_irq)

class tlcs90_device : public cpu_device
{
public:
	// construction/destruction
	tlcs90_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, address_map_constructor program_map);

	DECLARE_READ8_MEMBER( t90_internal_registers_r );
	DECLARE_WRITE8_MEMBER( t90_internal_registers_w );

	TIMER_CALLBACK_MEMBER( t90_timer_callback );
	TIMER_CALLBACK_MEMBER( t90_timer4_callback );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 2; }
	virtual UINT32 execute_max_cycles() const override { return 26; }
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual UINT32 execute_default_irq_vector() const override { return 0xff; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual void execute_burn(INT32 cycles) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr ); }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 6; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	enum e_mode {
		MODE_NONE,  MODE_BIT8,  MODE_CC,
		MODE_I8,    MODE_D8,    MODE_R8,
		MODE_I16,   MODE_D16,   MODE_R16,
		MODE_MI16,  MODE_MR16,  MODE_MR16D8,    MODE_MR16R8,
		MODE_R16D8, MODE_R16R8
	};

	address_space_config m_program_config;
	address_space_config m_io_config;

	PAIR        m_prvpc,m_pc,m_sp,m_af,m_bc,m_de,m_hl,m_ix,m_iy;
	PAIR        m_af2,m_bc2,m_de2,m_hl2;
	UINT8       m_halt, m_after_EI;
	UINT16      m_irq_state, m_irq_mask;
	address_space *m_program;
	address_space *m_io;
	int     m_icount;
	int         m_extra_cycles;       // extra cycles for interrupts
	UINT8       m_internal_registers[48];
	UINT32      m_ixbase,m_iybase;

	// Timers: 4 x 8-bit + 1 x 16-bit
	emu_timer   *m_timer[4+1];
	UINT8       m_timer_value[4];
	UINT16      m_timer4_value;
	attotime    m_timer_period;

	// Work registers
	UINT8        m_op;

	e_mode  m_mode1;
	UINT16  m_r1,m_r1b;

	e_mode  m_mode2;
	UINT16  m_r2,m_r2b;

	int m_cyc_t,m_cyc_f;

	UINT32  m_addr;

	inline UINT8  RM8 (UINT32 a);
	inline UINT16 RM16(UINT32 a);
	inline void WM8 (UINT32 a, UINT8  v);
	inline void WM16(UINT32 a, UINT16 v);
	inline UINT8  RX8 (UINT32 a, UINT32 base);
	inline UINT16 RX16(UINT32 a, UINT32 base);
	inline void WX8 (UINT32 a, UINT8  v, UINT32 base);
	inline void WX16(UINT32 a, UINT16 v, UINT32 base);
	inline UINT8  READ8();
	inline UINT16 READ16();
	void decode();
	const char *internal_registers_names(UINT16 x);
	int sprint_arg(char *buffer, UINT32 pc, const char *pre, const e_mode mode, const UINT16 r, const UINT16 rb);
	inline UINT16 r8( const UINT16 r );
	inline void w8( const UINT16 r, UINT16 value );
	inline UINT16 r16( const UINT16 r );
	inline void w16( const UINT16 r, UINT16 value );
	inline UINT8 Read1_8();
	inline UINT16 Read1_16();
	inline UINT8 Read2_8();
	inline UINT16 Read2_16();
	inline void Write1_8( UINT8 value );
	inline void Write1_16( UINT16 value );
	inline void Write2_8( UINT8 value );
	inline void Write2_16( UINT16 value );
	inline int Test( UINT8 cond );
	inline void Push( UINT16 rr );
	inline void Pop( UINT16 rr );
	inline void leave_halt();
	void take_interrupt(tlcs90_e_irq irq);
	void check_interrupts();
	inline void Cyc();
	inline void Cyc_f();
	void t90_start_timer(int i);
	void t90_start_timer4();
	void t90_stop_timer(int i);
	void t90_stop_timer4();
	void set_irq_line(int irq, int state);
};


class tmp90840_device : public tlcs90_device
{
public:
	// construction/destruction
	tmp90840_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tmp90841_device : public tlcs90_device
{
public:
	// construction/destruction
	tmp90841_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tmp91640_device : public tlcs90_device
{
public:
	// construction/destruction
	tmp91640_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tmp91641_device : public tlcs90_device
{
public:
	// construction/destruction
	tmp91641_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type TMP90840;
extern const device_type TMP90841;
extern const device_type TMP91640;
extern const device_type TMP91641;

#endif /* __TLCS90_H__ */
