// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#ifndef __PPS4_H__
#define __PPS4_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/
enum
{
	PPS4_PC,
	PPS4_A,
	PPS4_X,
	PPS4_SA,
	PPS4_SB,
	PPS4_B,
	PPS4_Skip,
	PPS4_SAG,
	PPS4_I1,
	PPS4_I2,
	PPS4_Ip,
	PPS4_GENPC = STATE_GENPC,
	PPS4_GENSP = STATE_GENSP,
	PPS4_GENPCBASE = STATE_GENPCBASE,
	PPS4_PORT_A = 256,
	PPS4_PORT_B = 257
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

extern const device_type PPS4;

class pps4_device : public cpu_device
{
public:
	// construction/destruction
	pps4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 3; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual UINT32 execute_default_irq_vector() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override
	{
		return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : ( (spacenum == AS_DATA) ? &m_data_config : nullptr ) );
	}

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	address_space *m_io;
	int     m_icount;

	UINT8   m_A;        //!< Accumulator A(4:1)
	UINT8   m_X;        //!< X register X(4:1)
	UINT16  m_P;        //!< program counter P(12:1)
	UINT16  m_SA;       //!< Shift register SA(12:1)
	UINT16  m_SB;       //!< Shift register SB(12:1)
	UINT8   m_Skip;     //!< Skip next instruction
	UINT16  m_SAG;      //!< Special address generation mask
	UINT16  m_B;        //!< B register B(12:1) (BL, BM and BH)
	UINT8   m_C;        //!< Carry flip-flop
	UINT8   m_FF1;      //!< Flip-flop 1
	UINT8   m_FF2;      //!< Flip-flop 2
	UINT8   m_I1;        //!< Most recent instruction I(8:1)
	UINT8   m_I2;       //!< Most recent parameter I2(8:1)
	UINT8   m_Ip;       //!< Previous instruction I(8:1)

	//! return memory at address B(12:1)
	inline UINT8 M();

	//! write to memory at address B(12:1)
	inline void W(UINT8 data);

	//! return the next opcode (also in m_I)
	inline UINT8 ROP();

	//! return the next argument (also in m_I2)
	inline UINT8 ARG();

	void iAD();          //!< Add
	void iADC();         //!< Add with carry-in
	void iADSK();        //!< Add and skip on carry-out
	void iADCSK();       //!< Add with carry-in and skip on carry-out
	void iADI();         //!< Add immediate
	void iDC();          //!< Decimal correction
	void iAND();         //!< Logical AND
	void iOR();          //!< Logical OR
	void iEOR();         //!< Logical Exclusive-OR
	void iCOMP();        //!< Complement
	void iSC();          //!< Set Carry flip-flop
	void iRC();          //!< Reset Carry flip-flop
	void iSF1();         //!< Set FF1
	void iRF1();         //!< Reset FF1
	void iSF2();         //!< Set FF2
	void iRF2();         //!< Reset FF2
	void iLD();          //!< Load accumulator from memory
	void iEX();          //!< Exchange accumulator and memory
	void iEXD();         //!< Exchange accumulator and memory and decrement BL
	void iLDI();         //!< Load accumulator immediate
	void iLAX();         //!< Load accumulator from X register
	void iLXA();         //!< Load X register from accumulator
	void iLABL();        //!< Load accumulator with BL
	void iLBMX();        //!< Load BM with X
	void iLBUA();        //!< Load BU with A
	void iXABL();        //!< Exchange accumulator and BL
	void iXBMX();        //!< Exchange BM and X registers
	void iXAX();         //!< Exchange accumulator and X
	void iXS();          //!< Eychange SA and SB registers
	void iCYS();         //!< Cycle SA register and accumulaor
	void iLB();          //!< Load B indirect
	void iLBL();         //!< Load B long
	void iINCB();        //!< Increment BL
	void iDECB();        //!< Decrement BL
	void iT();           //!< Transfer
	void iTM();          //!< Transfer and mark indirect
	void iTL();          //!< Transfer long
	void iTML();         //!< Transfer and mark long
	void iSKC();         //!< Skip on carry flip-flop
	void iSKZ();         //!< Skip on accumulator zero
	void iSKBI();        //!< Skip if BL equal to immediate
	void iSKF1();        //!< Skip if FF1 equals 1
	void iSKF2();        //!< Skip if FF2 equals 1
	void iRTN();         //!< Return
	void iRTNSK();       //!< Return and skip
	void iIOL();         //!< Input/Output long
	void iDIA();         //!< Discrete input group A
	void iDIB();         //!< Discrete input group B
	void iDOA();         //!< Discrete output group A
	void iSAG();         //!< Special address generation

	void execute_one(); //!< execute one instruction
};

#endif  // __PPS4_H__
