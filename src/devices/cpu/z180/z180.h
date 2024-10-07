// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#ifndef MAME_CPU_Z180_Z180_H
#define MAME_CPU_Z180_Z180_H

#pragma once

#include "z180asci.h"
#include "z180csio.h"

#include "machine/z80daisy.h"


enum
{
	Z180_PC,
	Z180_SP,
	Z180_AF,
	Z180_BC,
	Z180_DE,
	Z180_HL,
	Z180_IX,
	Z180_IY,
	Z180_A,
	Z180_B,
	Z180_C,
	Z180_D,
	Z180_E,
	Z180_H,
	Z180_L,
	Z180_AF2,
	Z180_BC2,
	Z180_DE2,
	Z180_HL2,
	Z180_R,
	Z180_I,
	Z180_IM,
	Z180_IFF1,
	Z180_IFF2,
	Z180_HALT,
	Z180_DC0,
	Z180_DC1,
	Z180_DC2,
	Z180_DC3,
	Z180_CNTLA0,
	Z180_CNTLA1,
	Z180_CNTLB0,
	Z180_CNTLB1,
	Z180_STAT0,
	Z180_STAT1,
	Z180_TDR0,
	Z180_TDR1,
	Z180_RDR0,
	Z180_RDR1,
	Z180_CNTR,
	Z180_TRDR,
	Z180_TMDR0,
	Z180_TMDR1,
	Z180_RLDR0,
	Z180_RLDR1,
	Z180_TCR,
	Z180_ASEXT0,
	Z180_ASEXT1,
	Z180_FRC,
	Z180_ASTC0,
	Z180_ASTC1,
	Z180_CMR,
	Z180_CCR,
	Z180_SAR0,
	Z180_DAR0,
	Z180_BCR0,
	Z180_MAR1,
	Z180_IAR1,
	Z180_BCR1,
	Z180_DSTAT,
	Z180_DMODE,
	Z180_DCNTL,
	Z180_IL,
	Z180_ITC,
	Z180_RCR,
	Z180_CBR,
	Z180_BBR,
	Z180_CBAR,
	Z180_OMCR,
	Z180_IOCR,
	Z180_IOLINES    /* read/write I/O lines */
};

enum
{
	Z180_TABLE_op,
	Z180_TABLE_cb,
	Z180_TABLE_ed,
	Z180_TABLE_xy,
	Z180_TABLE_xycb,
	Z180_TABLE_ex    // cycles counts for taken jr/jp/call and interrupt latency (rst opcodes) */
};

// input lines
enum {
	Z180_INPUT_LINE_IRQ0,           // Execute IRQ1
	Z180_INPUT_LINE_IRQ1,           // Execute IRQ1
	Z180_INPUT_LINE_IRQ2,           // Execute IRQ2
	Z180_INPUT_LINE_DREQ0,          // Start DMA0
	Z180_INPUT_LINE_DREQ1           // Start DMA1
};

class z180_device : public cpu_device, public z80_daisy_chain_interface
{
public:
	auto tend0_wr_callback() { return m_tend0_cb.bind(); }
	auto tend1_wr_callback() { return m_tend1_cb.bind(); }
	auto txa0_wr_callback() { return m_asci[0].lookup()->txa_handler(); }
	auto txa1_wr_callback() { return m_asci[1].lookup()->txa_handler(); }
	auto rts0_wr_callback() { return m_asci[0].lookup()->rts_handler(); }
	auto cka0_wr_callback() { return m_asci[0].lookup()->cka_handler(); }
	auto cka1_wr_callback() { return m_asci[1].lookup()->cka_handler(); }
	auto cks_wr_callback() { return m_csio.lookup()->cks_handler(); }
	auto txs_wr_callback() { return m_csio.lookup()->txs_handler(); }

	bool get_tend0();
	bool get_tend1();

	void rxa0_w(int state)     { m_asci[0]->rxa_wr(state); }
	void rxa1_w(int state)     { m_asci[1]->rxa_wr(state); }
	void cts0_w(int state)     { m_asci[0]->cts_wr(state); }
	void rxs_cts1_w(int state) { m_asci[1]->cts_wr(state); m_csio->rxs_wr(state); }
	void dcd0_w(int state)     { m_asci[0]->dcd_wr(state); }
	void cka0_w(int state)     { m_asci[0]->cka_wr(state); }
	void cka1_w(int state)     { m_asci[1]->cka_wr(state); }
	void cks_w(int state)      { m_csio->cks_wr(state); }

protected:
	// construction/destruction
	z180_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool extended_io, address_map_constructor internal_map);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_execute_interface implementation
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 16; }
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 2); }
	virtual uint32_t execute_default_irq_vector(int inputnum) const noexcept override { return 0xff; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_state_interface implementation
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint8_t z180_internal_port_read(uint8_t port);
	virtual void z180_internal_port_write(uint8_t port, uint8_t data);
	[[maybe_unused]] void burn_cycles(int32_t cycles);

	address_space_config m_program_config;
	address_space_config m_io_config;
	address_space_config m_decrypted_opcodes_config;
	required_device_array<z180asci_channel_base, 2> m_asci;
	required_device<z180csio_device> m_csio;

	void set_address_width(int bits);

private:
	uint8_t z180_read_memory(offs_t addr) { return m_program.read_byte(addr); }
	void z180_write_memory(offs_t addr, uint8_t data) { m_program.write_byte(addr, data); }

	int memory_wait_states() const { return (m_dcntl & 0xc0) >> 6; }
	int io_wait_states() const { return (m_dcntl & 0x30) == 0 ? 0 : ((m_dcntl & 0x30) >> 4) + 1; }
	bool is_internal_io_address(uint16_t port) const { return ((port ^ m_iocr) & (m_extended_io ? 0xff80 : 0xffc0)) == 0; }

	const bool m_extended_io;

	PAIR      m_PREPC,m_PC,m_SP,m_AF,m_BC,m_DE,m_HL,m_IX,m_IY;
	PAIR      m_AF2,m_BC2,m_DE2,m_HL2;
	uint8_t   m_R,m_R2,m_IFF1,m_IFF2,m_HALT,m_IM,m_I;
	uint8_t   m_tmdr_latch;                     // flag latched TMDR0H, TMDR1H values
	uint8_t   m_read_tcr_tmdr[2];               // flag to indicate that TCR or TMDR was read
	uint32_t  m_iol;                            // I/O line status bits
	PAIR16    m_tmdr[2];                        // PRT data register ch 0-1
	PAIR16    m_rldr[2];                        // PRT reload register ch 0-1
	uint8_t   m_tcr;                            // PRT control register
	uint8_t   m_frc;                            // free running counter (also time base for ASCI, CSI/O & PRT)
	uint8_t   m_frc_prescale;                   // divide CPU clock by 10
	PAIR      m_dma_sar0;                       // DMA source address register ch 0
	PAIR      m_dma_dar0;                       // DMA destination address register ch 0
	PAIR16    m_dma_bcr[2];                     // DMA byte register ch 0-1
	PAIR      m_dma_mar1;                       // DMA memory address register ch 1
	PAIR      m_dma_iar1;                       // DMA I/O address register ch 1
	uint8_t   m_dstat;                          // DMA status register
	uint8_t   m_dmode;                          // DMA mode register
	uint8_t   m_dcntl;                          // DMA/WAIT control register
	uint8_t   m_il;                             // INT vector low register
	uint8_t   m_itc;                            // INT/TRAP control register
	uint8_t   m_rcr;                            // refresh control register
	uint8_t   m_mmu_cbr;                        // MMU common base register
	uint8_t   m_mmu_bbr;                        // MMU bank base register
	uint8_t   m_mmu_cbar;                       // MMU common/bank area register
	uint8_t   m_omcr;                           // operation mode control register
	uint8_t   m_iocr;                           // I/O control register
	offs_t    m_mmu[16];                        // MMU address translation
	uint8_t   m_tmdrh[2];                       // latched TMDR0H and TMDR1H values
	uint16_t  m_tmdr_value[2];                  // TMDR values used byt PRT0 and PRT1 as down counter
	uint8_t   m_nmi_state;                      // NMI line state
	uint8_t   m_nmi_pending;                    // NMI pending
	uint8_t   m_irq_state[3];                   // IRQ line states (INT0,INT1,INT2)
	uint8_t   m_int_pending[11 + 1];            // interrupt pending
	uint8_t   m_after_EI;                       // are we in the EI shadow?
	uint32_t  m_ea;
	memory_access<20, 0, 0, ENDIANNESS_LITTLE>::cache m_cprogram, m_copcodes;
	memory_access<20, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_io;
	uint8_t   m_rtemp;
	uint32_t  m_ioltemp;
	int m_icount;
	int m_extra_cycles;           /* extra cpu cycles */
	uint8_t *m_cc[6];
	devcb_write_line m_tend0_cb, m_tend1_cb;

	typedef void (z180_device::*opcode_func)();
	static const opcode_func s_z180ops[6][0x100];

	inline void z180_mmu();
	inline u8 RM(offs_t addr);
	inline u8 IN(u16 port);
	inline void OUT(u16 port, u8 value);
	inline void RM16( offs_t addr, PAIR *r );
	inline void WM16( offs_t addr, PAIR *r );
	inline uint8_t ROP();
	inline uint8_t ARG();
	inline uint32_t ARG16();
	inline uint8_t INC(uint8_t value);
	inline uint8_t DEC(uint8_t value);
	inline uint8_t RLC(uint8_t value);
	inline uint8_t RRC(uint8_t value);
	inline uint8_t RL(uint8_t value);
	inline uint8_t RR(uint8_t value);
	inline uint8_t SLA(uint8_t value);
	inline uint8_t SRA(uint8_t value);
	inline uint8_t SLL(uint8_t value);
	inline uint8_t SRL(uint8_t value);
	inline uint8_t RES(uint8_t bit, uint8_t value);
	inline uint8_t SET(uint8_t bit, uint8_t value);
	inline int exec_op(const uint8_t opcode);
	inline int exec_cb(const uint8_t opcode);
	inline int exec_dd(const uint8_t opcode);
	inline int exec_ed(const uint8_t opcode);
	inline int exec_fd(const uint8_t opcode);
	inline int exec_xycb(const uint8_t opcode);
	int take_interrupt(int irq);
	uint8_t z180_readcontrol(offs_t port);
	void z180_writecontrol(offs_t port, uint8_t data);
	int z180_dma0(int max_cycles);
	int z180_dma1();
	void z180_write_iolines(uint32_t data);
	void clock_timers();
	int check_interrupts();
	void handle_io_timers(int cycles);

	void op_00();
	void op_01();
	void op_02();
	void op_03();
	void op_04();
	void op_05();
	void op_06();
	void op_07();
	void op_08();
	void op_09();
	void op_0a();
	void op_0b();
	void op_0c();
	void op_0d();
	void op_0e();
	void op_0f();
	void op_10();
	void op_11();
	void op_12();
	void op_13();
	void op_14();
	void op_15();
	void op_16();
	void op_17();
	void op_18();
	void op_19();
	void op_1a();
	void op_1b();
	void op_1c();
	void op_1d();
	void op_1e();
	void op_1f();
	void op_20();
	void op_21();
	void op_22();
	void op_23();
	void op_24();
	void op_25();
	void op_26();
	void op_27();
	void op_28();
	void op_29();
	void op_2a();
	void op_2b();
	void op_2c();
	void op_2d();
	void op_2e();
	void op_2f();
	void op_30();
	void op_31();
	void op_32();
	void op_33();
	void op_34();
	void op_35();
	void op_36();
	void op_37();
	void op_38();
	void op_39();
	void op_3a();
	void op_3b();
	void op_3c();
	void op_3d();
	void op_3e();
	void op_3f();
	void op_40();
	void op_41();
	void op_42();
	void op_43();
	void op_44();
	void op_45();
	void op_46();
	void op_47();
	void op_48();
	void op_49();
	void op_4a();
	void op_4b();
	void op_4c();
	void op_4d();
	void op_4e();
	void op_4f();
	void op_50();
	void op_51();
	void op_52();
	void op_53();
	void op_54();
	void op_55();
	void op_56();
	void op_57();
	void op_58();
	void op_59();
	void op_5a();
	void op_5b();
	void op_5c();
	void op_5d();
	void op_5e();
	void op_5f();
	void op_60();
	void op_61();
	void op_62();
	void op_63();
	void op_64();
	void op_65();
	void op_66();
	void op_67();
	void op_68();
	void op_69();
	void op_6a();
	void op_6b();
	void op_6c();
	void op_6d();
	void op_6e();
	void op_6f();
	void op_70();
	void op_71();
	void op_72();
	void op_73();
	void op_74();
	void op_75();
	void op_76();
	void op_77();
	void op_78();
	void op_79();
	void op_7a();
	void op_7b();
	void op_7c();
	void op_7d();
	void op_7e();
	void op_7f();
	void op_80();
	void op_81();
	void op_82();
	void op_83();
	void op_84();
	void op_85();
	void op_86();
	void op_87();
	void op_88();
	void op_89();
	void op_8a();
	void op_8b();
	void op_8c();
	void op_8d();
	void op_8e();
	void op_8f();
	void op_90();
	void op_91();
	void op_92();
	void op_93();
	void op_94();
	void op_95();
	void op_96();
	void op_97();
	void op_98();
	void op_99();
	void op_9a();
	void op_9b();
	void op_9c();
	void op_9d();
	void op_9e();
	void op_9f();
	void op_a0();
	void op_a1();
	void op_a2();
	void op_a3();
	void op_a4();
	void op_a5();
	void op_a6();
	void op_a7();
	void op_a8();
	void op_a9();
	void op_aa();
	void op_ab();
	void op_ac();
	void op_ad();
	void op_ae();
	void op_af();
	void op_b0();
	void op_b1();
	void op_b2();
	void op_b3();
	void op_b4();
	void op_b5();
	void op_b6();
	void op_b7();
	void op_b8();
	void op_b9();
	void op_ba();
	void op_bb();
	void op_bc();
	void op_bd();
	void op_be();
	void op_bf();
	void op_c0();
	void op_c1();
	void op_c2();
	void op_c3();
	void op_c4();
	void op_c5();
	void op_c6();
	void op_c7();
	void op_c8();
	void op_c9();
	void op_ca();
	void op_cb();
	void op_cc();
	void op_cd();
	void op_ce();
	void op_cf();
	void op_d0();
	void op_d1();
	void op_d2();
	void op_d3();
	void op_d4();
	void op_d5();
	void op_d6();
	void op_d7();
	void op_d8();
	void op_d9();
	void op_da();
	void op_db();
	void op_dc();
	void op_dd();
	void op_de();
	void op_df();
	void op_e0();
	void op_e1();
	void op_e2();
	void op_e3();
	void op_e4();
	void op_e5();
	void op_e6();
	void op_e7();
	void op_e8();
	void op_e9();
	void op_ea();
	void op_eb();
	void op_ec();
	void op_ed();
	void op_ee();
	void op_ef();
	void op_f0();
	void op_f1();
	void op_f2();
	void op_f3();
	void op_f4();
	void op_f5();
	void op_f6();
	void op_f7();
	void op_f8();
	void op_f9();
	void op_fa();
	void op_fb();
	void op_fc();
	void op_fd();
	void op_fe();
	void op_ff();
	void cb_00();
	void cb_01();
	void cb_02();
	void cb_03();
	void cb_04();
	void cb_05();
	void cb_06();
	void cb_07();
	void cb_08();
	void cb_09();
	void cb_0a();
	void cb_0b();
	void cb_0c();
	void cb_0d();
	void cb_0e();
	void cb_0f();
	void cb_10();
	void cb_11();
	void cb_12();
	void cb_13();
	void cb_14();
	void cb_15();
	void cb_16();
	void cb_17();
	void cb_18();
	void cb_19();
	void cb_1a();
	void cb_1b();
	void cb_1c();
	void cb_1d();
	void cb_1e();
	void cb_1f();
	void cb_20();
	void cb_21();
	void cb_22();
	void cb_23();
	void cb_24();
	void cb_25();
	void cb_26();
	void cb_27();
	void cb_28();
	void cb_29();
	void cb_2a();
	void cb_2b();
	void cb_2c();
	void cb_2d();
	void cb_2e();
	void cb_2f();
	void cb_30();
	void cb_31();
	void cb_32();
	void cb_33();
	void cb_34();
	void cb_35();
	void cb_36();
	void cb_37();
	void cb_38();
	void cb_39();
	void cb_3a();
	void cb_3b();
	void cb_3c();
	void cb_3d();
	void cb_3e();
	void cb_3f();
	void cb_40();
	void cb_41();
	void cb_42();
	void cb_43();
	void cb_44();
	void cb_45();
	void cb_46();
	void cb_47();
	void cb_48();
	void cb_49();
	void cb_4a();
	void cb_4b();
	void cb_4c();
	void cb_4d();
	void cb_4e();
	void cb_4f();
	void cb_50();
	void cb_51();
	void cb_52();
	void cb_53();
	void cb_54();
	void cb_55();
	void cb_56();
	void cb_57();
	void cb_58();
	void cb_59();
	void cb_5a();
	void cb_5b();
	void cb_5c();
	void cb_5d();
	void cb_5e();
	void cb_5f();
	void cb_60();
	void cb_61();
	void cb_62();
	void cb_63();
	void cb_64();
	void cb_65();
	void cb_66();
	void cb_67();
	void cb_68();
	void cb_69();
	void cb_6a();
	void cb_6b();
	void cb_6c();
	void cb_6d();
	void cb_6e();
	void cb_6f();
	void cb_70();
	void cb_71();
	void cb_72();
	void cb_73();
	void cb_74();
	void cb_75();
	void cb_76();
	void cb_77();
	void cb_78();
	void cb_79();
	void cb_7a();
	void cb_7b();
	void cb_7c();
	void cb_7d();
	void cb_7e();
	void cb_7f();
	void cb_80();
	void cb_81();
	void cb_82();
	void cb_83();
	void cb_84();
	void cb_85();
	void cb_86();
	void cb_87();
	void cb_88();
	void cb_89();
	void cb_8a();
	void cb_8b();
	void cb_8c();
	void cb_8d();
	void cb_8e();
	void cb_8f();
	void cb_90();
	void cb_91();
	void cb_92();
	void cb_93();
	void cb_94();
	void cb_95();
	void cb_96();
	void cb_97();
	void cb_98();
	void cb_99();
	void cb_9a();
	void cb_9b();
	void cb_9c();
	void cb_9d();
	void cb_9e();
	void cb_9f();
	void cb_a0();
	void cb_a1();
	void cb_a2();
	void cb_a3();
	void cb_a4();
	void cb_a5();
	void cb_a6();
	void cb_a7();
	void cb_a8();
	void cb_a9();
	void cb_aa();
	void cb_ab();
	void cb_ac();
	void cb_ad();
	void cb_ae();
	void cb_af();
	void cb_b0();
	void cb_b1();
	void cb_b2();
	void cb_b3();
	void cb_b4();
	void cb_b5();
	void cb_b6();
	void cb_b7();
	void cb_b8();
	void cb_b9();
	void cb_ba();
	void cb_bb();
	void cb_bc();
	void cb_bd();
	void cb_be();
	void cb_bf();
	void cb_c0();
	void cb_c1();
	void cb_c2();
	void cb_c3();
	void cb_c4();
	void cb_c5();
	void cb_c6();
	void cb_c7();
	void cb_c8();
	void cb_c9();
	void cb_ca();
	void cb_cb();
	void cb_cc();
	void cb_cd();
	void cb_ce();
	void cb_cf();
	void cb_d0();
	void cb_d1();
	void cb_d2();
	void cb_d3();
	void cb_d4();
	void cb_d5();
	void cb_d6();
	void cb_d7();
	void cb_d8();
	void cb_d9();
	void cb_da();
	void cb_db();
	void cb_dc();
	void cb_dd();
	void cb_de();
	void cb_df();
	void cb_e0();
	void cb_e1();
	void cb_e2();
	void cb_e3();
	void cb_e4();
	void cb_e5();
	void cb_e6();
	void cb_e7();
	void cb_e8();
	void cb_e9();
	void cb_ea();
	void cb_eb();
	void cb_ec();
	void cb_ed();
	void cb_ee();
	void cb_ef();
	void cb_f0();
	void cb_f1();
	void cb_f2();
	void cb_f3();
	void cb_f4();
	void cb_f5();
	void cb_f6();
	void cb_f7();
	void cb_f8();
	void cb_f9();
	void cb_fa();
	void cb_fb();
	void cb_fc();
	void cb_fd();
	void cb_fe();
	void cb_ff();
	void illegal_1();
	void dd_00();
	void dd_01();
	void dd_02();
	void dd_03();
	void dd_04();
	void dd_05();
	void dd_06();
	void dd_07();
	void dd_08();
	void dd_09();
	void dd_0a();
	void dd_0b();
	void dd_0c();
	void dd_0d();
	void dd_0e();
	void dd_0f();
	void dd_10();
	void dd_11();
	void dd_12();
	void dd_13();
	void dd_14();
	void dd_15();
	void dd_16();
	void dd_17();
	void dd_18();
	void dd_19();
	void dd_1a();
	void dd_1b();
	void dd_1c();
	void dd_1d();
	void dd_1e();
	void dd_1f();
	void dd_20();
	void dd_21();
	void dd_22();
	void dd_23();
	void dd_24();
	void dd_25();
	void dd_26();
	void dd_27();
	void dd_28();
	void dd_29();
	void dd_2a();
	void dd_2b();
	void dd_2c();
	void dd_2d();
	void dd_2e();
	void dd_2f();
	void dd_30();
	void dd_31();
	void dd_32();
	void dd_33();
	void dd_34();
	void dd_35();
	void dd_36();
	void dd_37();
	void dd_38();
	void dd_39();
	void dd_3a();
	void dd_3b();
	void dd_3c();
	void dd_3d();
	void dd_3e();
	void dd_3f();
	void dd_40();
	void dd_41();
	void dd_42();
	void dd_43();
	void dd_44();
	void dd_45();
	void dd_46();
	void dd_47();
	void dd_48();
	void dd_49();
	void dd_4a();
	void dd_4b();
	void dd_4c();
	void dd_4d();
	void dd_4e();
	void dd_4f();
	void dd_50();
	void dd_51();
	void dd_52();
	void dd_53();
	void dd_54();
	void dd_55();
	void dd_56();
	void dd_57();
	void dd_58();
	void dd_59();
	void dd_5a();
	void dd_5b();
	void dd_5c();
	void dd_5d();
	void dd_5e();
	void dd_5f();
	void dd_60();
	void dd_61();
	void dd_62();
	void dd_63();
	void dd_64();
	void dd_65();
	void dd_66();
	void dd_67();
	void dd_68();
	void dd_69();
	void dd_6a();
	void dd_6b();
	void dd_6c();
	void dd_6d();
	void dd_6e();
	void dd_6f();
	void dd_70();
	void dd_71();
	void dd_72();
	void dd_73();
	void dd_74();
	void dd_75();
	void dd_76();
	void dd_77();
	void dd_78();
	void dd_79();
	void dd_7a();
	void dd_7b();
	void dd_7c();
	void dd_7d();
	void dd_7e();
	void dd_7f();
	void dd_80();
	void dd_81();
	void dd_82();
	void dd_83();
	void dd_84();
	void dd_85();
	void dd_86();
	void dd_87();
	void dd_88();
	void dd_89();
	void dd_8a();
	void dd_8b();
	void dd_8c();
	void dd_8d();
	void dd_8e();
	void dd_8f();
	void dd_90();
	void dd_91();
	void dd_92();
	void dd_93();
	void dd_94();
	void dd_95();
	void dd_96();
	void dd_97();
	void dd_98();
	void dd_99();
	void dd_9a();
	void dd_9b();
	void dd_9c();
	void dd_9d();
	void dd_9e();
	void dd_9f();
	void dd_a0();
	void dd_a1();
	void dd_a2();
	void dd_a3();
	void dd_a4();
	void dd_a5();
	void dd_a6();
	void dd_a7();
	void dd_a8();
	void dd_a9();
	void dd_aa();
	void dd_ab();
	void dd_ac();
	void dd_ad();
	void dd_ae();
	void dd_af();
	void dd_b0();
	void dd_b1();
	void dd_b2();
	void dd_b3();
	void dd_b4();
	void dd_b5();
	void dd_b6();
	void dd_b7();
	void dd_b8();
	void dd_b9();
	void dd_ba();
	void dd_bb();
	void dd_bc();
	void dd_bd();
	void dd_be();
	void dd_bf();
	void dd_c0();
	void dd_c1();
	void dd_c2();
	void dd_c3();
	void dd_c4();
	void dd_c5();
	void dd_c6();
	void dd_c7();
	void dd_c8();
	void dd_c9();
	void dd_ca();
	void dd_cb();
	void dd_cc();
	void dd_cd();
	void dd_ce();
	void dd_cf();
	void dd_d0();
	void dd_d1();
	void dd_d2();
	void dd_d3();
	void dd_d4();
	void dd_d5();
	void dd_d6();
	void dd_d7();
	void dd_d8();
	void dd_d9();
	void dd_da();
	void dd_db();
	void dd_dc();
	void dd_dd();
	void dd_de();
	void dd_df();
	void dd_e0();
	void dd_e1();
	void dd_e2();
	void dd_e3();
	void dd_e4();
	void dd_e5();
	void dd_e6();
	void dd_e7();
	void dd_e8();
	void dd_e9();
	void dd_ea();
	void dd_eb();
	void dd_ec();
	void dd_ed();
	void dd_ee();
	void dd_ef();
	void dd_f0();
	void dd_f1();
	void dd_f2();
	void dd_f3();
	void dd_f4();
	void dd_f5();
	void dd_f6();
	void dd_f7();
	void dd_f8();
	void dd_f9();
	void dd_fa();
	void dd_fb();
	void dd_fc();
	void dd_fd();
	void dd_fe();
	void dd_ff();
	void illegal_2();
	void ed_00();
	void ed_01();
	void ed_02();
	void ed_03();
	void ed_04();
	void ed_05();
	void ed_06();
	void ed_07();
	void ed_08();
	void ed_09();
	void ed_0a();
	void ed_0b();
	void ed_0c();
	void ed_0d();
	void ed_0e();
	void ed_0f();
	void ed_10();
	void ed_11();
	void ed_12();
	void ed_13();
	void ed_14();
	void ed_15();
	void ed_16();
	void ed_17();
	void ed_18();
	void ed_19();
	void ed_1a();
	void ed_1b();
	void ed_1c();
	void ed_1d();
	void ed_1e();
	void ed_1f();
	void ed_20();
	void ed_21();
	void ed_22();
	void ed_23();
	void ed_24();
	void ed_25();
	void ed_26();
	void ed_27();
	void ed_28();
	void ed_29();
	void ed_2a();
	void ed_2b();
	void ed_2c();
	void ed_2d();
	void ed_2e();
	void ed_2f();
	void ed_30();
	void ed_31();
	void ed_32();
	void ed_33();
	void ed_34();
	void ed_35();
	void ed_36();
	void ed_37();
	void ed_38();
	void ed_39();
	void ed_3a();
	void ed_3b();
	void ed_3c();
	void ed_3d();
	void ed_3e();
	void ed_3f();
	void ed_40();
	void ed_41();
	void ed_42();
	void ed_43();
	void ed_44();
	void ed_45();
	void ed_46();
	void ed_47();
	void ed_48();
	void ed_49();
	void ed_4a();
	void ed_4b();
	void ed_4c();
	void ed_4d();
	void ed_4e();
	void ed_4f();
	void ed_50();
	void ed_51();
	void ed_52();
	void ed_53();
	void ed_54();
	void ed_55();
	void ed_56();
	void ed_57();
	void ed_58();
	void ed_59();
	void ed_5a();
	void ed_5b();
	void ed_5c();
	void ed_5d();
	void ed_5e();
	void ed_5f();
	void ed_60();
	void ed_61();
	void ed_62();
	void ed_63();
	void ed_64();
	void ed_65();
	void ed_66();
	void ed_67();
	void ed_68();
	void ed_69();
	void ed_6a();
	void ed_6b();
	void ed_6c();
	void ed_6d();
	void ed_6e();
	void ed_6f();
	void ed_70();
	void ed_71();
	void ed_72();
	void ed_73();
	void ed_74();
	void ed_75();
	void ed_76();
	void ed_77();
	void ed_78();
	void ed_79();
	void ed_7a();
	void ed_7b();
	void ed_7c();
	void ed_7d();
	void ed_7e();
	void ed_7f();
	void ed_80();
	void ed_81();
	void ed_82();
	void ed_83();
	void ed_84();
	void ed_85();
	void ed_86();
	void ed_87();
	void ed_88();
	void ed_89();
	void ed_8a();
	void ed_8b();
	void ed_8c();
	void ed_8d();
	void ed_8e();
	void ed_8f();
	void ed_90();
	void ed_91();
	void ed_92();
	void ed_93();
	void ed_94();
	void ed_95();
	void ed_96();
	void ed_97();
	void ed_98();
	void ed_99();
	void ed_9a();
	void ed_9b();
	void ed_9c();
	void ed_9d();
	void ed_9e();
	void ed_9f();
	void ed_a0();
	void ed_a1();
	void ed_a2();
	void ed_a3();
	void ed_a4();
	void ed_a5();
	void ed_a6();
	void ed_a7();
	void ed_a8();
	void ed_a9();
	void ed_aa();
	void ed_ab();
	void ed_ac();
	void ed_ad();
	void ed_ae();
	void ed_af();
	void ed_b0();
	void ed_b1();
	void ed_b2();
	void ed_b3();
	void ed_b4();
	void ed_b5();
	void ed_b6();
	void ed_b7();
	void ed_b8();
	void ed_b9();
	void ed_ba();
	void ed_bb();
	void ed_bc();
	void ed_bd();
	void ed_be();
	void ed_bf();
	void ed_c0();
	void ed_c1();
	void ed_c2();
	void ed_c3();
	void ed_c4();
	void ed_c5();
	void ed_c6();
	void ed_c7();
	void ed_c8();
	void ed_c9();
	void ed_ca();
	void ed_cb();
	void ed_cc();
	void ed_cd();
	void ed_ce();
	void ed_cf();
	void ed_d0();
	void ed_d1();
	void ed_d2();
	void ed_d3();
	void ed_d4();
	void ed_d5();
	void ed_d6();
	void ed_d7();
	void ed_d8();
	void ed_d9();
	void ed_da();
	void ed_db();
	void ed_dc();
	void ed_dd();
	void ed_de();
	void ed_df();
	void ed_e0();
	void ed_e1();
	void ed_e2();
	void ed_e3();
	void ed_e4();
	void ed_e5();
	void ed_e6();
	void ed_e7();
	void ed_e8();
	void ed_e9();
	void ed_ea();
	void ed_eb();
	void ed_ec();
	void ed_ed();
	void ed_ee();
	void ed_ef();
	void ed_f0();
	void ed_f1();
	void ed_f2();
	void ed_f3();
	void ed_f4();
	void ed_f5();
	void ed_f6();
	void ed_f7();
	void ed_f8();
	void ed_f9();
	void ed_fa();
	void ed_fb();
	void ed_fc();
	void ed_fd();
	void ed_fe();
	void ed_ff();
	void fd_00();
	void fd_01();
	void fd_02();
	void fd_03();
	void fd_04();
	void fd_05();
	void fd_06();
	void fd_07();
	void fd_08();
	void fd_09();
	void fd_0a();
	void fd_0b();
	void fd_0c();
	void fd_0d();
	void fd_0e();
	void fd_0f();
	void fd_10();
	void fd_11();
	void fd_12();
	void fd_13();
	void fd_14();
	void fd_15();
	void fd_16();
	void fd_17();
	void fd_18();
	void fd_19();
	void fd_1a();
	void fd_1b();
	void fd_1c();
	void fd_1d();
	void fd_1e();
	void fd_1f();
	void fd_20();
	void fd_21();
	void fd_22();
	void fd_23();
	void fd_24();
	void fd_25();
	void fd_26();
	void fd_27();
	void fd_28();
	void fd_29();
	void fd_2a();
	void fd_2b();
	void fd_2c();
	void fd_2d();
	void fd_2e();
	void fd_2f();
	void fd_30();
	void fd_31();
	void fd_32();
	void fd_33();
	void fd_34();
	void fd_35();
	void fd_36();
	void fd_37();
	void fd_38();
	void fd_39();
	void fd_3a();
	void fd_3b();
	void fd_3c();
	void fd_3d();
	void fd_3e();
	void fd_3f();
	void fd_40();
	void fd_41();
	void fd_42();
	void fd_43();
	void fd_44();
	void fd_45();
	void fd_46();
	void fd_47();
	void fd_48();
	void fd_49();
	void fd_4a();
	void fd_4b();
	void fd_4c();
	void fd_4d();
	void fd_4e();
	void fd_4f();
	void fd_50();
	void fd_51();
	void fd_52();
	void fd_53();
	void fd_54();
	void fd_55();
	void fd_56();
	void fd_57();
	void fd_58();
	void fd_59();
	void fd_5a();
	void fd_5b();
	void fd_5c();
	void fd_5d();
	void fd_5e();
	void fd_5f();
	void fd_60();
	void fd_61();
	void fd_62();
	void fd_63();
	void fd_64();
	void fd_65();
	void fd_66();
	void fd_67();
	void fd_68();
	void fd_69();
	void fd_6a();
	void fd_6b();
	void fd_6c();
	void fd_6d();
	void fd_6e();
	void fd_6f();
	void fd_70();
	void fd_71();
	void fd_72();
	void fd_73();
	void fd_74();
	void fd_75();
	void fd_76();
	void fd_77();
	void fd_78();
	void fd_79();
	void fd_7a();
	void fd_7b();
	void fd_7c();
	void fd_7d();
	void fd_7e();
	void fd_7f();
	void fd_80();
	void fd_81();
	void fd_82();
	void fd_83();
	void fd_84();
	void fd_85();
	void fd_86();
	void fd_87();
	void fd_88();
	void fd_89();
	void fd_8a();
	void fd_8b();
	void fd_8c();
	void fd_8d();
	void fd_8e();
	void fd_8f();
	void fd_90();
	void fd_91();
	void fd_92();
	void fd_93();
	void fd_94();
	void fd_95();
	void fd_96();
	void fd_97();
	void fd_98();
	void fd_99();
	void fd_9a();
	void fd_9b();
	void fd_9c();
	void fd_9d();
	void fd_9e();
	void fd_9f();
	void fd_a0();
	void fd_a1();
	void fd_a2();
	void fd_a3();
	void fd_a4();
	void fd_a5();
	void fd_a6();
	void fd_a7();
	void fd_a8();
	void fd_a9();
	void fd_aa();
	void fd_ab();
	void fd_ac();
	void fd_ad();
	void fd_ae();
	void fd_af();
	void fd_b0();
	void fd_b1();
	void fd_b2();
	void fd_b3();
	void fd_b4();
	void fd_b5();
	void fd_b6();
	void fd_b7();
	void fd_b8();
	void fd_b9();
	void fd_ba();
	void fd_bb();
	void fd_bc();
	void fd_bd();
	void fd_be();
	void fd_bf();
	void fd_c0();
	void fd_c1();
	void fd_c2();
	void fd_c3();
	void fd_c4();
	void fd_c5();
	void fd_c6();
	void fd_c7();
	void fd_c8();
	void fd_c9();
	void fd_ca();
	void fd_cb();
	void fd_cc();
	void fd_cd();
	void fd_ce();
	void fd_cf();
	void fd_d0();
	void fd_d1();
	void fd_d2();
	void fd_d3();
	void fd_d4();
	void fd_d5();
	void fd_d6();
	void fd_d7();
	void fd_d8();
	void fd_d9();
	void fd_da();
	void fd_db();
	void fd_dc();
	void fd_dd();
	void fd_de();
	void fd_df();
	void fd_e0();
	void fd_e1();
	void fd_e2();
	void fd_e3();
	void fd_e4();
	void fd_e5();
	void fd_e6();
	void fd_e7();
	void fd_e8();
	void fd_e9();
	void fd_ea();
	void fd_eb();
	void fd_ec();
	void fd_ed();
	void fd_ee();
	void fd_ef();
	void fd_f0();
	void fd_f1();
	void fd_f2();
	void fd_f3();
	void fd_f4();
	void fd_f5();
	void fd_f6();
	void fd_f7();
	void fd_f8();
	void fd_f9();
	void fd_fa();
	void fd_fb();
	void fd_fc();
	void fd_fd();
	void fd_fe();
	void fd_ff();
	void xycb_00();
	void xycb_01();
	void xycb_02();
	void xycb_03();
	void xycb_04();
	void xycb_05();
	void xycb_06();
	void xycb_07();
	void xycb_08();
	void xycb_09();
	void xycb_0a();
	void xycb_0b();
	void xycb_0c();
	void xycb_0d();
	void xycb_0e();
	void xycb_0f();
	void xycb_10();
	void xycb_11();
	void xycb_12();
	void xycb_13();
	void xycb_14();
	void xycb_15();
	void xycb_16();
	void xycb_17();
	void xycb_18();
	void xycb_19();
	void xycb_1a();
	void xycb_1b();
	void xycb_1c();
	void xycb_1d();
	void xycb_1e();
	void xycb_1f();
	void xycb_20();
	void xycb_21();
	void xycb_22();
	void xycb_23();
	void xycb_24();
	void xycb_25();
	void xycb_26();
	void xycb_27();
	void xycb_28();
	void xycb_29();
	void xycb_2a();
	void xycb_2b();
	void xycb_2c();
	void xycb_2d();
	void xycb_2e();
	void xycb_2f();
	void xycb_30();
	void xycb_31();
	void xycb_32();
	void xycb_33();
	void xycb_34();
	void xycb_35();
	void xycb_36();
	void xycb_37();
	void xycb_38();
	void xycb_39();
	void xycb_3a();
	void xycb_3b();
	void xycb_3c();
	void xycb_3d();
	void xycb_3e();
	void xycb_3f();
	void xycb_40();
	void xycb_41();
	void xycb_42();
	void xycb_43();
	void xycb_44();
	void xycb_45();
	void xycb_46();
	void xycb_47();
	void xycb_48();
	void xycb_49();
	void xycb_4a();
	void xycb_4b();
	void xycb_4c();
	void xycb_4d();
	void xycb_4e();
	void xycb_4f();
	void xycb_50();
	void xycb_51();
	void xycb_52();
	void xycb_53();
	void xycb_54();
	void xycb_55();
	void xycb_56();
	void xycb_57();
	void xycb_58();
	void xycb_59();
	void xycb_5a();
	void xycb_5b();
	void xycb_5c();
	void xycb_5d();
	void xycb_5e();
	void xycb_5f();
	void xycb_60();
	void xycb_61();
	void xycb_62();
	void xycb_63();
	void xycb_64();
	void xycb_65();
	void xycb_66();
	void xycb_67();
	void xycb_68();
	void xycb_69();
	void xycb_6a();
	void xycb_6b();
	void xycb_6c();
	void xycb_6d();
	void xycb_6e();
	void xycb_6f();
	void xycb_70();
	void xycb_71();
	void xycb_72();
	void xycb_73();
	void xycb_74();
	void xycb_75();
	void xycb_76();
	void xycb_77();
	void xycb_78();
	void xycb_79();
	void xycb_7a();
	void xycb_7b();
	void xycb_7c();
	void xycb_7d();
	void xycb_7e();
	void xycb_7f();
	void xycb_80();
	void xycb_81();
	void xycb_82();
	void xycb_83();
	void xycb_84();
	void xycb_85();
	void xycb_86();
	void xycb_87();
	void xycb_88();
	void xycb_89();
	void xycb_8a();
	void xycb_8b();
	void xycb_8c();
	void xycb_8d();
	void xycb_8e();
	void xycb_8f();
	void xycb_90();
	void xycb_91();
	void xycb_92();
	void xycb_93();
	void xycb_94();
	void xycb_95();
	void xycb_96();
	void xycb_97();
	void xycb_98();
	void xycb_99();
	void xycb_9a();
	void xycb_9b();
	void xycb_9c();
	void xycb_9d();
	void xycb_9e();
	void xycb_9f();
	void xycb_a0();
	void xycb_a1();
	void xycb_a2();
	void xycb_a3();
	void xycb_a4();
	void xycb_a5();
	void xycb_a6();
	void xycb_a7();
	void xycb_a8();
	void xycb_a9();
	void xycb_aa();
	void xycb_ab();
	void xycb_ac();
	void xycb_ad();
	void xycb_ae();
	void xycb_af();
	void xycb_b0();
	void xycb_b1();
	void xycb_b2();
	void xycb_b3();
	void xycb_b4();
	void xycb_b5();
	void xycb_b6();
	void xycb_b7();
	void xycb_b8();
	void xycb_b9();
	void xycb_ba();
	void xycb_bb();
	void xycb_bc();
	void xycb_bd();
	void xycb_be();
	void xycb_bf();
	void xycb_c0();
	void xycb_c1();
	void xycb_c2();
	void xycb_c3();
	void xycb_c4();
	void xycb_c5();
	void xycb_c6();
	void xycb_c7();
	void xycb_c8();
	void xycb_c9();
	void xycb_ca();
	void xycb_cb();
	void xycb_cc();
	void xycb_cd();
	void xycb_ce();
	void xycb_cf();
	void xycb_d0();
	void xycb_d1();
	void xycb_d2();
	void xycb_d3();
	void xycb_d4();
	void xycb_d5();
	void xycb_d6();
	void xycb_d7();
	void xycb_d8();
	void xycb_d9();
	void xycb_da();
	void xycb_db();
	void xycb_dc();
	void xycb_dd();
	void xycb_de();
	void xycb_df();
	void xycb_e0();
	void xycb_e1();
	void xycb_e2();
	void xycb_e3();
	void xycb_e4();
	void xycb_e5();
	void xycb_e6();
	void xycb_e7();
	void xycb_e8();
	void xycb_e9();
	void xycb_ea();
	void xycb_eb();
	void xycb_ec();
	void xycb_ed();
	void xycb_ee();
	void xycb_ef();
	void xycb_f0();
	void xycb_f1();
	void xycb_f2();
	void xycb_f3();
	void xycb_f4();
	void xycb_f5();
	void xycb_f6();
	void xycb_f7();
	void xycb_f8();
	void xycb_f9();
	void xycb_fa();
	void xycb_fb();
	void xycb_fc();
	void xycb_fd();
	void xycb_fe();
	void xycb_ff();
};

class z80180_device : public z180_device
{
public:
	// construction/destruction
	z80180_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class hd64180rp_device : public z180_device
{
public:
	// construction/destruction
	hd64180rp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class z8s180_device : public z180_device
{
public:
	// construction/destruction
	z8s180_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	z8s180_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_execute_interface implementation
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return BIT(m_cmr, 7) ? (clocks * 2) : BIT(m_ccr, 7) ? clocks : (clocks + 2 - 1) / 2; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return BIT(m_cmr, 7) ? (cycles + 2 - 1) / 2 : BIT(m_ccr, 7) ? cycles : (cycles * 2); }

	virtual uint8_t z180_internal_port_read(uint8_t port) override;
	virtual void z180_internal_port_write(uint8_t port, uint8_t data) override;

private:
	uint8_t   m_cmr;                            // clock multiplier
	uint8_t   m_ccr;                            // chip control register
};

class z80182_device : public z8s180_device
{
public:
	// construction/destruction
	z80182_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(Z80180, z80180_device)
DECLARE_DEVICE_TYPE(HD64180RP, hd64180rp_device)
DECLARE_DEVICE_TYPE(Z8S180, z8s180_device)
DECLARE_DEVICE_TYPE(Z80182, z80182_device)

#endif // MAME_CPU_Z180_Z180_H
