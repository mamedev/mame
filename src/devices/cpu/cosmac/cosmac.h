// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA COSMAC CPU emulation

**********************************************************************
                            _____   _____
                   Vcc   1 |*    \_/     | 40  Vdd
                _BUS 3   2 |             | 39  _BUS 4
                _BUS 2   3 |             | 38  _BUS 5
                _BUS 1   4 |             | 37  _BUS 6
                _BUS 0   5 |             | 36  _BUS 7
                   _N0   6 |             | 35  Vss
                   _N1   7 |             | 34  _EF1
                   _N2   8 |             | 33  _EF2
                   _N3   9 |             | 32  _EF3
                     *  10 |   CDP1801U  | 31  _EF4
                     *  11 |             | 30  _DMA OUT
                     *  12 |             | 29  _INTERRUPT
                     *  13 |             | 28  _DMA IN
                     *  14 |             | 27  _CLEAR
                _CLOCK  15 |             | 26  _LOAD
                  _TPB  16 |             | 25  _SC2
                  _TPA  17 |             | 24  _SC1
                     *  18 |             | 23  _SC0
                   MWR  19 |             | 22  _M READ
                   Vss  20 |_____________| 21  *

                            _____   _____
                   Vcc   1 |*    \_/     | 28  Vdd
                _BUS 4   2 |             | 27  _BUS 3
                _BUS 5   3 |             | 26  _BUS 2
                _BUS 6   4 |             | 25  _BUS 1
                _BUS 7   5 |             | 24  _BUS 0
                  _MA0   6 |             | 23  *
                  _MA1   7 |   CDP1801C  | 22  _TPB
                  _MA2   8 |             | 21  *
                  _MA3   9 |             | 20  *
                  _MA4  10 |             | 19  *
                  _MA5  11 |             | 18  *
                  _MA6  12 |             | 17  *
                  _MA7  13 |             | 16  *
                   Vss  14 |_____________| 15  _CLEAR

                            _____   _____
                 CLOCK   1 |*    \_/     | 40  Vdd
                 _WAIT   2 |             | 39  _XTAL
                _CLEAR   3 |             | 38  _DMA IN
                     Q   4 |             | 37  _DMA OUT
                   SC1   5 |             | 36  _INTERRUPT
                   SC0   6 |             | 35  _MWR
                  _MRD   7 |             | 34  TPA
                 BUS 7   8 |             | 33  TPB
                 BUS 6   9 |   CDP1802   | 32  MA7
                 BUS 5  10 |   CDP1803   | 31  MA6
                 BUS 4  11 |   CDP1804   | 30  MA5
                 BUS 3  12 |   CDP1805   | 29  MA4
                 BUS 2  13 |   CDP1806   | 28  MA3
                 BUS 1  14 |             | 27  MA2
                 BUS 0  15 |             | 26  MA1
                     *  16 |             | 25  MA0
                    N2  17 |             | 24  _EF1
                    N1  18 |             | 23  _EF2
                    N0  19 |             | 22  _EF3
                   Vss  20 |_____________| 21  _EF4


    Type            Internal ROM    Internal RAM    Timer   Pin 16 (*)
    ------------------------------------------------------------------
    CDP1802         none            none            no          Vcc
    CDP1803         ?               ?               ?           ?
    CDP1804         2 KB            64 bytes        yes         ?
    CDP1805         none            64 bytes        yes         _ME
    CDP1806         none            none            yes         Vdd

**********************************************************************/

#pragma once

#ifndef __COSMAC_H__
#define __COSMAC_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_COSMAC_WAIT_CALLBACK(_read) \
	devcb = &cosmac_device::set_wait_rd_callback(*device, DEVCB_##_read);

#define MCFG_COSMAC_CLEAR_CALLBACK(_read) \
	devcb = &cosmac_device::set_clear_rd_callback(*device, DEVCB_##_read);

#define MCFG_COSMAC_EF1_CALLBACK(_read) \
	devcb = &cosmac_device::set_ef1_rd_callback(*device, DEVCB_##_read);

#define MCFG_COSMAC_EF2_CALLBACK(_read) \
	devcb = &cosmac_device::set_ef2_rd_callback(*device, DEVCB_##_read);

#define MCFG_COSMAC_EF3_CALLBACK(_read) \
	devcb = &cosmac_device::set_ef3_rd_callback(*device, DEVCB_##_read);

#define MCFG_COSMAC_EF4_CALLBACK(_read) \
	devcb = &cosmac_device::set_ef4_rd_callback(*device, DEVCB_##_read);

#define MCFG_COSMAC_Q_CALLBACK(_write) \
	devcb = &cosmac_device::set_q_wr_callback(*device, DEVCB_##_write);

#define MCFG_COSMAC_DMAR_CALLBACK(_read) \
	devcb = &cosmac_device::set_dma_rd_callback(*device, DEVCB_##_read);

#define MCFG_COSMAC_DMAW_CALLBACK(_write) \
	devcb = &cosmac_device::set_dma_wr_callback(*device, DEVCB_##_write);

#define MCFG_COSMAC_SC_CALLBACK(_write) \
	devcb = &cosmac_device::set_sc_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  ENUMERATIONS
//**************************************************************************

// registers
enum
{
	COSMAC_P,
	COSMAC_X,
	COSMAC_D,
	COSMAC_B,
	COSMAC_T,
	COSMAC_R0,
	COSMAC_R1,
	COSMAC_R2,
	COSMAC_R3,
	COSMAC_R4,
	COSMAC_R5,
	COSMAC_R6,
	COSMAC_R7,
	COSMAC_R8,
	COSMAC_R9,
	COSMAC_Ra,
	COSMAC_Rb,
	COSMAC_Rc,
	COSMAC_Rd,
	COSMAC_Re,
	COSMAC_Rf,
	COSMAC_DF,
	COSMAC_IE,
	COSMAC_Q,
	COSMAC_N,
	COSMAC_I,
	COSMAC_SC
};


// input lines
enum
{
	COSMAC_INPUT_LINE_INT = 0,
	COSMAC_INPUT_LINE_DMAIN,
	COSMAC_INPUT_LINE_DMAOUT,
	COSMAC_INPUT_LINE_EF1,
	COSMAC_INPUT_LINE_EF2,
	COSMAC_INPUT_LINE_EF3,
	COSMAC_INPUT_LINE_EF4
};


// state codes
enum cosmac_state_code
{
	COSMAC_STATE_CODE_S0_FETCH = 0,
	COSMAC_STATE_CODE_S1_EXECUTE,
	COSMAC_STATE_CODE_S2_DMA,
	COSMAC_STATE_CODE_S3_INTERRUPT
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cosmac_device

class cosmac_device : public cpu_device
{
public:
	// construction/destruction
	cosmac_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	template<class _Object> static devcb_base &set_wait_rd_callback(device_t &device, _Object object) { return downcast<cosmac_device &>(device).m_read_wait.set_callback(object); }
	template<class _Object> static devcb_base &set_clear_rd_callback(device_t &device, _Object object) { return downcast<cosmac_device &>(device).m_read_clear.set_callback(object); }
	template<class _Object> static devcb_base &set_ef1_rd_callback(device_t &device, _Object object) { return downcast<cosmac_device &>(device).m_read_ef1.set_callback(object); }
	template<class _Object> static devcb_base &set_ef2_rd_callback(device_t &device, _Object object) { return downcast<cosmac_device &>(device).m_read_ef2.set_callback(object); }
	template<class _Object> static devcb_base &set_ef3_rd_callback(device_t &device, _Object object) { return downcast<cosmac_device &>(device).m_read_ef3.set_callback(object); }
	template<class _Object> static devcb_base &set_ef4_rd_callback(device_t &device, _Object object) { return downcast<cosmac_device &>(device).m_read_ef4.set_callback(object); }
	template<class _Object> static devcb_base &set_q_wr_callback(device_t &device, _Object object) { return downcast<cosmac_device &>(device).m_write_q.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_rd_callback(device_t &device, _Object object) { return downcast<cosmac_device &>(device).m_read_dma.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_wr_callback(device_t &device, _Object object) { return downcast<cosmac_device &>(device).m_write_dma.set_callback(object); }
	template<class _Object> static devcb_base &set_sc_wr_callback(device_t &device, _Object object) { return downcast<cosmac_device &>(device).m_write_sc.set_callback(object); }

	// public interfaces
	offs_t get_memory_address();

	DECLARE_WRITE_LINE_MEMBER( int_w ) { set_input_line(COSMAC_INPUT_LINE_INT, state); }
	DECLARE_WRITE_LINE_MEMBER( dma_in_w ) { set_input_line(COSMAC_INPUT_LINE_DMAIN, state); }
	DECLARE_WRITE_LINE_MEMBER( dma_out_w ) { set_input_line(COSMAC_INPUT_LINE_DMAOUT, state); }
	DECLARE_WRITE_LINE_MEMBER( ef1_w ) { set_input_line(COSMAC_INPUT_LINE_EF1, state); }
	DECLARE_WRITE_LINE_MEMBER( ef2_w ) { set_input_line(COSMAC_INPUT_LINE_EF2, state); }
	DECLARE_WRITE_LINE_MEMBER( ef3_w ) { set_input_line(COSMAC_INPUT_LINE_EF3, state); }
	DECLARE_WRITE_LINE_MEMBER( ef4_w ) { set_input_line(COSMAC_INPUT_LINE_EF4, state); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override;
	virtual UINT32 execute_max_cycles() const override;
	virtual UINT32 execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override;
	virtual UINT32 disasm_max_opcode_bytes() const override;

	// helpers
	inline UINT8 read_opcode(offs_t pc);
	inline UINT8 read_byte(offs_t address);
	inline UINT8 read_io_byte(offs_t address);
	inline void write_byte(offs_t address, UINT8 data);
	inline void write_io_byte(offs_t address, UINT8 data);

	// execution logic
	inline void run();
	inline void debug();
	inline void reset();
	inline void initialize();
	inline void fetch_instruction();
	inline void fetch_instruction_debug();
	inline void execute_instruction();
	inline void dma_input();
	inline void dma_output();
	inline void interrupt();
	inline void sample_wait_clear();
	inline void sample_ef_lines();
	inline void output_state_code();
	inline void set_q_flag(int state);

	// arithmetic handlers
	void add(int left, int right);
	void add_with_carry(int left, int right);
	void subtract(int left, int right);
	void subtract_with_borrow(int left, int right);

	// condition handlers
	void short_branch(int taken);
	void long_branch(int taken);
	void long_skip(int taken);

	// control handlers
	void return_from_interrupt(int ie);

	// memory reference opcode handlers
	void ldn();
	void lda();
	void ldx();
	void ldxa();
	void ldi();
	void str();
	void stxd();

	// register operations opcode handlers
	void inc();
	void dec();
	void irx();
	void glo();
	void plo();
	void ghi();
	void phi();

	// logic operations opcode handlers
	void _or();
	void ori();
	void _xor();
	void xri();
	void _and();
	void ani();
	void shr();
	void shrc();
	void shl();
	void shlc();

	// arithmetic operations opcode handlers
	void add();
	void adi();
	void adc();
	void adci();
	void sd();
	void sdi();
	void sdb();
	void sdbi();
	void sm();
	void smi();
	void smb();
	void smbi();

	// short branch instructions opcode handlers
	void br();
	void nbr();
	void bz();
	void bnz();
	void bdf();
	void bnf();
	void bq();
	void bnq();
	void b();
	void bn();

	// long branch instructions opcode handlers
	void lbr();
	void nlbr();
	void lbz();
	void lbnz();
	void lbdf();
	void lbnf();
	void lbq();
	void lbnq();

	// skip instructions opcode handlers
	void lsz();
	void lsnz();
	void lsdf();
	void lsnf();
	void lsq();
	void lsnq();
	void lsie();

	// control instructions opcode handlers
	void idl();
	void nop();
	void und();
	void sep();
	void sex();
	void seq();
	void req();
	void sav();
	void mark();
	void ret();
	void dis();

	// input/output byte transfer opcode handlers
	void out();
	void inp();

	const address_space_config      m_program_config;
	const address_space_config      m_io_config;

	// device callbacks
	devcb_read_line        m_read_wait;
	devcb_read_line        m_read_clear;
	devcb_read_line        m_read_ef1;
	devcb_read_line        m_read_ef2;
	devcb_read_line        m_read_ef3;
	devcb_read_line        m_read_ef4;
	devcb_write_line       m_write_q;
	devcb_read8            m_read_dma;
	devcb_write8           m_write_dma;
	devcb_write8           m_write_sc;

	// control modes
	enum cosmac_mode
	{
		COSMAC_MODE_LOAD = 0,
		COSMAC_MODE_RESET,
		COSMAC_MODE_PAUSE,
		COSMAC_MODE_RUN
	};

	// execution states
	enum cosmac_state
	{
		COSMAC_STATE_0_FETCH = 0,
		COSMAC_STATE_1_RESET,
		COSMAC_STATE_1_INIT,
		COSMAC_STATE_1_EXECUTE,
		COSMAC_STATE_2_DMA_IN,
		COSMAC_STATE_2_DMA_OUT,
		COSMAC_STATE_3_INT
	};

	// internal state
	UINT16              m_pc;               // fake program counter
	UINT8               m_op;               // current opcode
	UINT8               m_flagsio;          // flags storage for state saving
	cosmac_state        m_state;            // state
	cosmac_mode         m_mode;             // control mode
	cosmac_mode         m_pmode;            // previous control mode
	int                 m_irq;              // interrupt request
	int                 m_dmain;            // DMA input request
	int                 m_dmaout;           // DMA output request
	int                 m_ef[4];            // external flags

	// registers
	UINT8               m_d;                // data register (accumulator)
	UINT8               m_b;                // auxiliary holding register
	UINT16              m_r[16];            // scratchpad registers
	UINT8               m_p;                // designates which register is Program Counter
	UINT8               m_x;                // designates which register is Data Pointer
	UINT8               m_n;                // low-order instruction digit
	UINT8               m_i;                // high-order instruction digit
	UINT8               m_t;                // temporary register

	// flags
	int                 m_df;               // data flag (ALU carry)
	int                 m_ie;               // interrupt enable
	int                 m_q;                // output flip-flop

	// internal stuff
	int                 m_icount;
	address_space *     m_program;
	address_space *     m_io;
	direct_read_data *  m_direct;

	// opcode/condition tables
	typedef void (cosmac_device::*ophandler)();
	virtual cosmac_device::ophandler get_ophandler(UINT8 opcode) = 0;
};


// ======================> cdp1801_device

class cdp1801_device : public cosmac_device
{
public:
	// construction/destruction
	cdp1801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device_disasm_interface overrides
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual cosmac_device::ophandler get_ophandler(UINT8 opcode) override;

	static const ophandler s_opcodetable[256];
};


// ======================> cdp1802_device

class cdp1802_device : public cosmac_device
{
public:
	// construction/destruction
	cdp1802_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device_disasm_interface overrides
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual cosmac_device::ophandler get_ophandler(UINT8 opcode) override;

	static const ophandler s_opcodetable[256];
};


// device type definition
extern const device_type CDP1801;
extern const device_type CDP1802;


#endif /* __COSMAC_H__ */
