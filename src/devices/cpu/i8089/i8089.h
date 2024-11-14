// license:GPL-2.0+
// copyright-holders:Dirk Best,Carl
/***************************************************************************

    Intel 8089 I/O Processor

***************************************************************************/

#ifndef MAME_CPU_I8089_I8089_H
#define MAME_CPU_I8089_I8089_H

#pragma once

#ifdef _MSC_VER
// MSVC seems to want to actually instantiate templates when it gets an extern template declaration, effectively defeating the purpose of extern template declatations altogether
// In this case it causes a problem because the required_device template can't be instantiated for the incomplete i8089_channel_device type
#include "i8089_channel.h"
#endif


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class i8089_channel_device;

// ======================> i8089_device

class i8089_device : public cpu_device
{
	friend class i8089_channel_device;

public:
	// construction/destruction
	i8089_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto sintr1() { return m_write_sintr1.bind(); }
	auto sintr2() { return m_write_sintr2.bind(); }

	// configuration helpers
	void set_data_width(uint8_t data_width) { m_data_width = data_width; }

	// input lines
	void ca_w(int state);
	void sel_w(int state) { m_sel = state; }
	void drq1_w(int state);
	void drq2_w(int state);
	void ext1_w(int state);
	void ext2_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;

	int m_icount;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_program_config;
	address_space_config m_io_config;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	bool sysbus_width() const { return BIT(m_sysbus, 0); }
	bool remotebus_width() const { return BIT(m_soc, 0); }
	bool request_grant() const { return BIT(m_soc, 1); }

	// internal communication
	void ch1_sintr_w(int state) { m_write_sintr1(state); }
	void ch2_sintr_w(int state) { m_write_sintr2(state); }

	uint8_t read_byte(bool space, offs_t address);
	uint16_t read_word(bool space, offs_t address);
	void write_byte(bool space, offs_t address, uint8_t data);
	void write_word(bool space, offs_t address, uint16_t data);

	required_device<i8089_channel_device> m_ch1;
	required_device<i8089_channel_device> m_ch2;

	devcb_write_line m_write_sintr1;
	devcb_write_line m_write_sintr2;

	void initialize();

	uint8_t m_data_width;
	address_space *m_mem;
	address_space *m_io;

	// register indexes for the debugger state
	enum
	{
		SYSBUS,
		SCB,
		SOC,
		DIVIDER1,
		CH1_GA, CH1_GB, CH1_GC,
		CH1_TP, CH1_BC, CH1_IX,
		CH1_CC, CH1_MC, CH1_CP,
		CH1_PP, CH1_PSW,
		DIVIDER2,
		CH2_GA, CH2_GB, CH2_GC,
		CH2_TP, CH2_BC, CH2_IX,
		CH2_CC, CH2_MC, CH2_CP,
		CH2_PP, CH2_PSW
	};

	// system configuration
	uint8_t m_sysbus;
	offs_t m_scb;
	uint8_t m_soc;

	bool m_initialized;
	bool m_master;

	// task pointer for the currently executing channel
	offs_t m_current_tp;

	// state of input pins
	int m_ca;
	int m_sel;
	bool m_last_chan;
};


// device type definition
DECLARE_DEVICE_TYPE(I8089, i8089_device)

#endif // MAME_CPU_I8089_I8089_H
