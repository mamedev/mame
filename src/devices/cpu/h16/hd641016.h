// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************
                                     Vss
     A11/D10 -----------------------+ | +---------------------- A10/D9
     A12/D11 ---------------------+ | | | +-------------------- A9/D8
     A13/D12 -------------------+ | | | | | +------------------ A8/D7
     A14/D13 -----------------+ | | | | | | | +---------------- A7/D6
     A15/D14 ---------------+ | | | | | | | | | +-------------- A6/D5
     A16/D15 -------------+ | | | | | | | | | | | +------------ A5/D4
         A17 -----------+ | | | | | | | | | | | | | +---------- A4/D3
         A18 ---------+ | | | | | | | | | | | | | | | +-------- A3/D2
         A19 -------+ | | | | | | | | | | | | | | | | | +------ A2/D1
         A20 -----+ | | | | | | | | | | | | | | | | | | | +---- A1/D0
                 _1_1_|_|_|_|_|_|_|_|_|_8_8_8_8_8_7_7_7_7_7_
                / 1 0 9 8 7 6 5 4 3 2 1 4 3 2 1 0 9 8 7 6 5 \
               /--------------------------------------------|
         A21 --| 12                   *                  74 |-- _IRQ1
         A22 --| 13                                      73 |-- _IRQ0
         A23 --| 14                                      72 |-- _NMI
         Vss --| 15                                      71 |-- _BRTRY
         _AS --| 16                                      70 |-- _RES
       _WAIT --| 17                                      69 |-- Vss
         Vcc --| 18                                      68 |-- EXTAL
        _HDS --| 19                                      67 |-- (NC)
        _LDS --| 20                                      66 |-- XTAL
        R/_W --| 21                                      65 |-- _BREQ
        S/_U --| 22               HD641016CP             64 |-- (NC)
         Vss --| 23                                      63 |-- Vcc
          PF --| 24                                      62 |-- E
        (NC) --| 25                                      61 |-- ϕ
        (NC) --| 26                                      60 |-- _BACK
       TIOB2 --| 27                                      59 |-- _IACK
       TIOA2 --| 28                                      58 |-- PCS1
       TIOB1 --| 29                                      57 |-- PCS0
       TIOA1 --| 30                                      56 |-- ST2
        (NC) --| 31                                      55 |-- ST1
        RXD0 --| 32                                      54 |-- ST0
               |--------------------------------------------|
               \  3 3 3 3 3 3 3 4 4 4 4 4 4 4 4 4 4 5 5 5 5 /
                --3-4-5-6-7-8-9-0-1-2-3-4-5-6-7-8-9-0-1-2-3-
        TXD0 -----+ | | | | | | | | | | | | | | | | | | | +---- _DONE
        RXC0 -------+ | | | | | | | | | | | | | | | | | +------ _DACK0
        TXC0 ---------+ | | | | | | | | | | | | | | | +-------- _DREQ0
       _CTS0 -----------+ | | | | | | | | | | | | | +---------- _DACK1
       _DCD0 -------------+ | | | | | | | | | | | +------------ _DREQ1
       _RTS0 ---------------+ | | | | | | | | | +-------------- _DACK2
        RXD1 -----------------+ | | | | | | | +---------------- _DREQ2
        TXD1 -------------------+ | | | | | +------------------ _RTS1/_DACK3
        RXC1 ---------------------+ | | | +-------------------- _DCD1/_DREQ3
        TXC1 -----------------------+ | +---------------------- Vss
                                    _CTS1

****************************************************************************/

#ifndef MAME_CPU_H16_HD641016_H
#define MAME_CPU_H16_HD641016_H

#pragma once

class hd641016_device : public cpu_device
{
public:
	enum
	{
		H16_PC,
		H16_SSP,
		H16_BSP,
		H16_EBR,
		H16_RBR,
		H16_IBR,
		H16_CBNR,
		H16_SR,
		H16_CCR,
		H16_BMR,
		H16_GBNR,
		H16_VBNR,
		H16_R0, H16_R1, H16_R2, H16_R3, H16_R4, H16_R5, H16_R6, H16_R7,
		H16_R8, H16_R9, H16_R10, H16_R11, H16_R12, H16_R13, H16_R14,
		H16_USP,
		H16_CR0, H16_CR1, H16_CR2, H16_CR3, H16_CR4, H16_CR5, H16_CR6, H16_CR7,
		H16_CR8, H16_CR9, H16_CR10, H16_CR11, H16_CR12, H16_CR13, H16_CR14, H16_CR15,
		H16_PR0, H16_PR1, H16_PR2, H16_PR3, H16_PR4, H16_PR5, H16_PR6, H16_PR7,
		H16_PR8, H16_PR9, H16_PR10, H16_PR11, H16_PR12, H16_PR13, H16_PR14, H16_PR15
	};

	// device type constructor
	hd641016_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 2); }
	virtual void execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	u8 global_bank_mask() const { return BIT(m_bmr, 4) ? 7 : (2 << (m_bmr & 3)) - 1; }

	// internal maps
	void ram_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	// address spaces
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;
	memory_access<24, 1, 0, ENDIANNESS_BIG>::specific m_program;
	memory_access<24, 1, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<10, 2, 0, ENDIANNESS_BIG>::cache m_data;
	memory_access<9, 2, 0, ENDIANNESS_BIG>::specific m_io;

	u32 m_pc;
	u32 m_ssp;
	u32 m_bsp;
	u32 m_ebr;
	u32 m_rbr;
	u32 m_ibr;
	u32 m_cbnr;
	u16 m_sr;
	u8 m_bmr;
	u8 m_gbnr;
	u8 m_vbnr;
	s32 m_icount;
};

// device type declaration
DECLARE_DEVICE_TYPE(HD641016, hd641016_device)

#endif // MAME_CPU_H16_HD641016_H
