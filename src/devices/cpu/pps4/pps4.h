// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#ifndef MAME_CPU_PPS4_PPS4_H
#define MAME_CPU_PPS4_PPS4_H

#pragma once


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
	PPS4_Ip
};


//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

DECLARE_DEVICE_TYPE(PPS4,   pps4_device)
DECLARE_DEVICE_TYPE(PPS4_2, pps4_2_device)

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

class pps4_device : public cpu_device
{
public:
	// construction/destruction
	pps4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto dia_cb() { return m_dia_cb.bind(); }
	auto dib_cb() { return m_dib_cb.bind(); }
	auto do_cb() { return m_do_cb.bind(); }

	u16 address_bus_r(address_space &space);

protected:
	pps4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 3; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	devcb_read8 m_dia_cb;
	devcb_read8 m_dib_cb;
	devcb_write8 m_do_cb;

	memory_access<12, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<12, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<12, 0, 0, ENDIANNESS_LITTLE>::specific m_data;
	memory_access< 8, 0, 0, ENDIANNESS_LITTLE>::specific m_io;
	int     m_icount;
	int     m_wasldi;
	int     m_waslbl;

	u8        m_A;        //!< Accumulator A(4:1)
	u8        m_X;        //!< X register X(4:1)
	u16       m_P;        //!< program counter P(12:1)
	u16       m_SA;       //!< Shift register SA(12:1)
	u16       m_SB;       //!< Shift register SB(12:1)
	u8        m_Skip;     //!< Skip next instruction
	u16       m_SAG;      //!< Special address generation mask
	u16       m_B;        //!< B register B(12:1) (BL, BM and BH)
	u8        m_C;        //!< Carry flip-flop
	u8        m_FF1;      //!< Flip-flop 1
	u8        m_FF2;      //!< Flip-flop 2
	u8        m_I1;        //!< Most recent instruction I(8:1)
	u8        m_I2;       //!< Most recent parameter I2(8:1)

	//! return memory at address B(12:1)
	inline u8 M();

	//! write to memory at address B(12:1)
	inline void W(u8 data);

	//! return the next opcode (also in m_I)
	inline u8 ROP();

	//! return the next argument (also in m_I2)
	inline u8 ARG();

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
	void iCYS();         //!< Cycle SA register and accumulator
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
	virtual void iDIB(); //!< Discrete input group B
	virtual void iDOA(); //!< Discrete output group A
	void iSAG();         //!< Special address generation

	void execute_one(); //!< execute one instruction
};

class pps4_2_device : public pps4_device
{
public:
	// construction/destruction
	pps4_2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides (NOTE: these assume internal XTAL divider is always used)
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 18 - 1) / 18; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 18); }

	virtual void iDIB() override;
	virtual void iDOA() override;

private:
	u8        m_DIO;      //!< DIO clamp
};

#endif // MAME_CPU_PPS4_PPS4_H
