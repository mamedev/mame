// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Harris HD-6120 High-Speed CMOS 12 Bit Microprocessor

***********************************************************************
                            _____   _____
                  _OUT   1 |*    \_/     | 40  VCC
                DMAGNT   2 |             | 39  _READ
               _DMAREQ   3 |             | 38  _WRITE
                 _SKIP   4 |             | 37  _MEMSEL
              RUN/_HLT   5 |             | 36  _IOCLR
                  _RUN   6 |             | 35  _LXDAR
                _RESET   7 |             | 34  _LXMAR
                   ACK   8 |             | 33  _LXPAR
                 OSCIN   9 |             | 32  _DATAF
                OSCOUT  10 |   HD-6120   | 31  _INTGNT
               _IFETCH  11 |             | 30  _INTREQ
                   DX0  12 |             | 29  _CPREQ
                   DX1  13 |             | 28  STRTUP
                   DX2  14 |             | 27  EMA2
                   DX3  15 |             | 26  C1/_C1
                   DX4  16 |             | 25  C0/_C0
                   DX5  17 |             | 24  DX11
                   DX6  18 |             | 23  DX10
                   DX7  19 |             | 22  DX9
                   VSS  20 |_____________| 21  DX8

***************************************************************************/

#ifndef MAME_CPU_PDP8_HD6120_H
#define MAME_CPU_PDP8_HD6120_H

#pragma once

class hd6120_device : public cpu_device
{
public:
	static constexpr int AS_DEVCTL = AS_OPCODES + 1;

	static constexpr offs_t INSTF = 0;
	static constexpr offs_t IFETCH = 1;
	static constexpr offs_t DATAF = 2;

	enum {
		HD6120_PC,
		HD6120_AC, HD6120_MQ,
		HD6120_SP1, HD6120_SP2,
		HD6120_IF, HD6120_IB, HD6120_DF, HD6120_SF, HD6120_IIFF,
		HD6120_FLAGS, HD6120_PNLFLGS, HD6120_PWRON
	};

	// input lines
	enum {
		INTREQ_LINE = 0,
		CPREQ_LINE
		//SKIP_LINE,
		//DMAREQ_LINE
	};

	// device control flags
	enum : u8 {
		SKIP = 1 << 3,
		C0 = 1 << 2,
		C1 = 1 << 1
		// C2 is ignored on HD-6120
	};

	// device type constructor
	hd6120_device(const machine_config &config, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto lxmar_callback() { return m_lxmar_callback.bind(); }
	auto lxpar_callback() { return m_lxpar_callback.bind(); }
	auto lxdar_callback() { return m_lxdar_callback.bind(); }
	auto rsr_callback() { return m_rsr_callback.bind(); }
	auto wsr_callback() { return m_wsr_callback.bind(); }
	auto strtup_callback() { return m_strtup_callback.bind(); }
	auto intgnt_callback() { return m_intgnt_callback.bind(); }
	auto ioclr_callback() { return m_ioclr_callback.bind(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual void execute_run() override;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 2); }
	virtual void execute_set_input(int linenum, int state) override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	enum class minor_state : u8 {
		RESET_1, RESET_2, RESET_3, RESET_4, RESET_5,
		IFETCH_1, IFETCH_2, IFETCH_3,
		INDIR_1, INDIR_2, INDIR_3, INDIR_3A, INDIR_4, INDIR_5,
		EXEC_1, EXEC_2, EXEC_3,
		DEP_2, DEP_3,
		AND_4,
		TAD_4,
		ISZ_4, ISZ_5, ISZ_6, ISZ_7, ISZ_8,
		DCA_4,
		JMS_4,
		JMP_1,
		OP1_1, OP1_2, OP1_3, OP1_4, OP1_5,
		OP2_1, OP2_2, OP2_3, OP2_4,
		OSR_2, OSR_3,
		OP3_1, OP3_2, OP3_3,
		IOT_1, IOT_2,
		SKON_1, SKON_2, SKON_3,
		IEN_1, IEN_2,
		GTF_1, GTF_2, GTF_3, GTF_4, GTF_5,
		RTF_1, RTF_2, RTF_3, RTF_4,
		SRQ_1,
		SGT_1,
		CAF_1, CAF_2, CAF_3,
		PRS_1, PRS_2, PRS_3, PRS_4,
		PGO_1,
		PEX_1, PEX_2,
		CFIELD_1, CFIELD_2,
		RFIELD_1, RFIELD_2,
		RIB_1, RIB_2,
		RMF_1, RMF_2, RMF_3, RMF_4,
		PRQ_1,
		WSR_1, WSR_2,
		GCF_1, GCF_2, GCF_3, GCF_4,
		SPD_1,
		PPC_1, PPC_2, PPC_3, PPC_4, PPC_5, PPC_6,
		PAC_1, PAC_2, PAC_3,
		RTN_1, RTN_2, RTN_3, RTN_4,
		POP_1, POP_2, POP_3, POP_4,
		RSP_1, RSP_2,
		LSP_1,
		EXTIOT_1, EXTIOT_2, EXTIOT_3, EXTIOT_4, EXTIOT_4R, EXTIOT_5R, EXTIOT_5,
		INTGNT_1,
		CPINT_1, CPINT_2
	};

	u16 rotate_step(u16 data);
	bool skip_test() const;
	u16 dataf_map(u16 addr) const;
	void next_instruction();
	void transfer_pc(u16 addr);
	void debug_set_pc(u16 addr);
	void debug_update_pc(u16 addr);

	// address spaces
	address_space_config m_inst_config;
	address_space_config m_data_config;
	address_space_config m_io_config;
	address_space_config m_devctl_config;
	memory_access<16, 1, -1, ENDIANNESS_BIG>::cache m_icache;
	memory_access<16, 1, -1, ENDIANNESS_BIG>::specific m_inst;
	memory_access<16, 1, -1, ENDIANNESS_BIG>::specific m_data;
	memory_access<9, 1, -1, ENDIANNESS_BIG>::specific m_io;
	memory_access<9, 0, 0, ENDIANNESS_BIG>::specific m_devctl;

	// callback objects
	devcb_write16 m_lxmar_callback;
	devcb_write16 m_lxpar_callback;
	devcb_write16 m_lxdar_callback;
	devcb_read16 m_rsr_callback;
	devcb_write16 m_wsr_callback;
	//devcb_read16 m_rtin_callback;
	//devcb_write16 m_rtout_callback;
	devcb_read_line m_strtup_callback;
	devcb_write_line m_intgnt_callback;
	//devcb_write_line m_dmagnt_callback;
	devcb_write_line m_ioclr_callback;

	// major registers
	u16 m_pc;
	u16 m_ac;
	u16 m_mq;
	u16 m_sp[2];
	u16 m_temp;
	u16 m_ir;

	// field and flag registers
	u8 m_if;
	u8 m_ib;
	u8 m_df;
	u8 m_sf;
	u8 m_flags;
	u8 m_pnlflgs;
	bool m_fz;
	bool m_iiff;
	bool m_pwron;
	bool m_intgnt;

	// misc. execution state
	minor_state m_state;
	u16 m_iaddr;
	u16 m_oaddr;
	s32 m_icount;

	// input lines
	bool m_intreq_input;
	bool m_cpreq_input;
};

// device type declaration
DECLARE_DEVICE_TYPE(HD6120, hd6120_device)

#endif // MAME_CPU_PDP8_HD6120_H
