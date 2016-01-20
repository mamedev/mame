// license:GPL-2.0+
// copyright-holders:Dirk Best,Carl
/***************************************************************************

    Intel 8089 I/O Processor

***************************************************************************/

#pragma once

#ifndef __I8089_H__
#define __I8089_H__

#include "emu.h"
#include "i8089_channel.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_I8089_DATABUS_WIDTH(_databus_width) \
	i8089_device::set_databus_width(*device, _databus_width);

#define MCFG_I8089_SINTR1(_sintr1) \
	downcast<i8089_device *>(device)->set_sintr1_callback(DEVCB_##_sintr1);

#define MCFG_I8089_SINTR2(_sintr2) \
	downcast<i8089_device *>(device)->set_sintr2_callback(DEVCB_##_sintr2);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class i8089_channel;

// ======================> i8089_device

class i8089_device : public cpu_device
{
	friend class i8089_channel;

public:
	// construction/destruction
	i8089_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// callbacks
	template<class _sintr1> void set_sintr1_callback(_sintr1 sintr1) { m_write_sintr1.set_callback(sintr1); }
	template<class _sintr2> void set_sintr2_callback(_sintr2 sintr2) { m_write_sintr2.set_callback(sintr2); }

	// static configuration helpers
	static void set_databus_width(device_t &device, UINT8 databus_width) { downcast<i8089_device &>(device).m_databus_width = databus_width; }

	// input lines
	DECLARE_WRITE_LINE_MEMBER( ca_w );
	DECLARE_WRITE_LINE_MEMBER( sel_w ) { m_sel = state; }
	DECLARE_WRITE_LINE_MEMBER( drq1_w );
	DECLARE_WRITE_LINE_MEMBER( drq2_w );
	DECLARE_WRITE_LINE_MEMBER( ext1_w );
	DECLARE_WRITE_LINE_MEMBER( ext2_w );

	// internal communication
	DECLARE_WRITE_LINE_MEMBER( ch1_sintr_w ) { m_write_sintr1(state); }
	DECLARE_WRITE_LINE_MEMBER( ch2_sintr_w ) { m_write_sintr2(state); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_run() override;

	int m_icount;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	address_space_config m_program_config;
	address_space_config m_io_config;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 7; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	bool sysbus_width() const { return BIT(m_sysbus, 0); }
	bool remotebus_width() const { return BIT(m_soc, 0); }
	bool request_grant() const { return BIT(m_soc, 1); }

	UINT8 read_byte(bool space, offs_t address);
	UINT16 read_word(bool space, offs_t address);
	void write_byte(bool space, offs_t address, UINT8 data);
	void write_word(bool space, offs_t address, UINT16 data);

	required_device<i8089_channel> m_ch1;
	required_device<i8089_channel> m_ch2;

	devcb_write_line m_write_sintr1;
	devcb_write_line m_write_sintr2;

	void initialize();

	UINT8 m_databus_width;
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
	UINT8 m_sysbus;
	offs_t m_scb;
	UINT8 m_soc;

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
extern const device_type I8089;


#endif  /* __I8089_H__ */
