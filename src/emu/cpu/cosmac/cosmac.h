/**********************************************************************

    RCA "COSMAC" CPU emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
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
	COSMAC_INPUT_LINE_EF4,
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

typedef void (*cosmac_out_sc_func)(device_t *device, cosmac_state_code sc);
#define COSMAC_SC_WRITE(name) void name(device_t *device, cosmac_state_code sc)


// ======================> cosmac_interface

struct cosmac_interface
{
	devcb_read_line     m_in_wait_cb;
	devcb_read_line     m_in_clear_cb;
	devcb_read_line     m_in_ef1_cb;
	devcb_read_line     m_in_ef2_cb;
	devcb_read_line     m_in_ef3_cb;
	devcb_read_line     m_in_ef4_cb;
	devcb_write_line    m_out_q_cb;
	devcb_read8         m_in_dma_cb;
	devcb_write8        m_out_dma_cb;
	cosmac_out_sc_func  m_out_sc_cb;
	devcb_write_line    m_out_tpa_cb;
	devcb_write_line    m_out_tpb_cb;
};

#define COSMAC_INTERFACE(name) \
	const cosmac_interface (name) =


// ======================> cosmac_device

class cosmac_device : public cpu_device,
						public cosmac_interface
{
public:
	// construction/destruction
	cosmac_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// public interfaces
	offs_t get_memory_address();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const;
	virtual UINT32 execute_max_cycles() const;
	virtual UINT32 execute_input_lines() const;
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	virtual void state_string_export(const device_state_entry &entry, astring &string);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

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
	devcb_resolved_read_line    m_in_wait_func;
	devcb_resolved_read_line    m_in_clear_func;
	devcb_resolved_read_line    m_in_ef_func[4];
	devcb_resolved_write_line   m_out_q_func;
	devcb_resolved_read8        m_in_dma_func;
	devcb_resolved_write8       m_out_dma_func;
	cosmac_out_sc_func          m_out_sc_func;
	devcb_resolved_write_line   m_out_tpa_func;
	devcb_resolved_write_line   m_out_tpb_func;

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

	static const ophandler s_opcodetable[256];
};


// device type definition
extern const device_type COSMAC;


#endif /* __COSMAC_H__ */
