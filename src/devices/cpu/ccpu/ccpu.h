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

	// configuration helpers
	auto external_func() { return m_external_input.bind(); }

	template <typename... T> void set_vector_func(T &&... args) { m_vector_callback.set(std::forward<T>(args)...); }

	uint8_t read_jmi();
	void wdt_timer_trigger();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; }
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

	memory_access<15, 0,  0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<15, 0,  0, ENDIANNESS_BIG>::specific m_program;
	memory_access<32, 1, -1, ENDIANNESS_BIG>::specific m_data;
	memory_access< 5, 0,  0, ENDIANNESS_BIG>::specific m_io;

	uint16_t m_flags;
};


DECLARE_DEVICE_TYPE(CCPU, ccpu_cpu_device)

#endif // MAME_CPU_CCPU_CCPU_H
