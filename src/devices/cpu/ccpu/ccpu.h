// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ccpu.h
    Core implementation for the portable Cinematronics CPU emulator.

    Written by Aaron Giles
    Special thanks to Zonn Moore for his detailed documentation.

***************************************************************************/

#pragma once

#ifndef __CCPU_H__
#define __CCPU_H__


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	CCPU_PC=1,
	CCPU_FLAGS,
	CCPU_A,
	CCPU_B,
	CCPU_I,
	CCPU_J,
	CCPU_P,
	CCPU_X,
	CCPU_Y,
	CCPU_T
};


typedef device_delegate<void (INT16, INT16, INT16, INT16, UINT8)> ccpu_vector_delegate;


#define MCFG_CCPU_EXTERNAL_FUNC(_devcb) \
	ccpu_cpu_device::set_external_func(*device, DEVCB_##_devcb);

#define MCFG_CCPU_VECTOR_FUNC(d) \
	ccpu_cpu_device::set_vector_func(*device, d);


class ccpu_cpu_device : public cpu_device
{
public:
	// construction/destruction
	ccpu_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_external_func(device_t &device, _Object object) { return downcast<ccpu_cpu_device &>(device).m_external_input.set_callback(object); }
	static void set_vector_func(device_t &device, ccpu_vector_delegate callback) { downcast<ccpu_cpu_device &>(device).m_vector_callback = callback; }

	DECLARE_READ8_MEMBER( read_jmi );
	void wdt_timer_trigger();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 1; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override
	{
		switch (spacenum)
		{
			case AS_PROGRAM: return &m_program_config;
			case AS_IO:      return &m_io_config;
			case AS_DATA:    return &m_data_config;
			default:         return nullptr;
		}
	}

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 3; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	UINT16              m_PC;
	UINT16              m_A;
	UINT16              m_B;
	UINT8               m_I;
	UINT16              m_J;
	UINT8               m_P;
	UINT16              m_X;
	UINT16              m_Y;
	UINT16              m_T;
	UINT16 *            m_acc;

	UINT16              m_a0flag, m_ncflag, m_cmpacc, m_cmpval;
	UINT16              m_miflag, m_nextmiflag, m_nextnextmiflag;
	UINT16              m_drflag;

	devcb_read8        m_external_input;
	ccpu_vector_delegate m_vector_callback;

	UINT8               m_waiting;
	UINT8               m_watchdog;
	UINT8               m_extinput;

	int                 m_icount;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	address_space *m_io;

	UINT16 m_flags;
};


extern const device_type CCPU;


#endif /* __CCPU_H__ */
