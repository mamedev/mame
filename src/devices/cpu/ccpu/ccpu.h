// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ccpu.h
    Core implementation for the portable Cinematronics CPU emulator.

    Written by Aaron Giles
    Special thanks to Zonn Moore for his detailed documentation.

***************************************************************************/

#ifndef MAME_CPU_CCPU_CCPU_H
#define MAME_CPU_CCPU_CCPU_H

#pragma once


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/



#define MCFG_CCPU_EXTERNAL_FUNC(_devcb) \
	ccpu_cpu_device::set_external_func(*device, DEVCB_##_devcb);

#define MCFG_CCPU_VECTOR_FUNC(d) \
	ccpu_cpu_device::set_vector_func(*device, d);


class ccpu_cpu_device : public cpu_device
{
public:
	// register enumeration
	// public because the cinemat driver accesses A/P/X/Y through state interace - should there be a proper public interface to read registers?
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

	typedef device_delegate<void (int16_t, int16_t, int16_t, int16_t, uint8_t)> vector_delegate;

	// construction/destruction
	ccpu_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template <class Object> static devcb_base &set_external_func(device_t &device, Object &&cb) { return downcast<ccpu_cpu_device &>(device).m_external_input.set_callback(std::forward<Object>(cb)); }
	static void set_vector_func(device_t &device, vector_delegate callback) { downcast<ccpu_cpu_device &>(device).m_vector_callback = callback; }

	DECLARE_READ8_MEMBER( read_jmi );
	void wdt_timer_trigger();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 1; }
	virtual uint32_t execute_input_lines() const override { return 0; }
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
	virtual uint32_t disasm_min_opcode_bytes() const override { return 1; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 3; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	uint16_t              m_PC;
	uint16_t              m_A;
	uint16_t              m_B;
	uint8_t               m_I;
	uint16_t              m_J;
	uint8_t               m_P;
	uint16_t              m_X;
	uint16_t              m_Y;
	uint16_t              m_T;
	uint16_t *            m_acc;

	uint16_t              m_a0flag, m_ncflag, m_cmpacc, m_cmpval;
	uint16_t              m_miflag, m_nextmiflag, m_nextnextmiflag;
	uint16_t              m_drflag;

	devcb_read8        m_external_input;
	vector_delegate m_vector_callback;

	uint8_t               m_waiting;
	uint8_t               m_watchdog;
	uint8_t               m_extinput;

	int                 m_icount;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	address_space *m_io;

	uint16_t m_flags;
};


DECLARE_DEVICE_TYPE(CCPU, ccpu_cpu_device)

#endif // MAME_CPU_CCPU_CCPU_H
