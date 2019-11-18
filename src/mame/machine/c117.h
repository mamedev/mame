// license:BSD-3-Clause
// copyright-holders:Alex W. Jackson
#ifndef MAME_MACHINE_C117_H
#define MAME_MACHINE_C117_H

#pragma once


#include "machine/watchdog.h"


//***************************************************************************
//  TYPE DEFINITIONS
//***************************************************************************

class namco_c117_device :
	public device_t,
	public device_memory_interface
{
public:
	//construction/destruction
	namco_c117_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T, typename U> void set_cpu_tags(T &&maintag, U &&subtag)
	{
		m_cpuexec[0].set_tag(std::forward<T>(maintag));
		m_cpuexec[1].set_tag(std::forward<U>(subtag));
	}
	auto subres_cb() { return m_subres_cb.bind(); }

	DECLARE_READ8_MEMBER(main_r);
	DECLARE_READ8_MEMBER(sub_r);
	DECLARE_WRITE8_MEMBER(main_w);
	DECLARE_WRITE8_MEMBER(sub_w);

	// FIXME: this doesn't belong here
	DECLARE_WRITE8_MEMBER(sound_watchdog_w);

	offs_t remap(int whichcpu, offs_t offset) { return m_offsets[whichcpu][offset>>13] | (offset & 0x1fff); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// internal helpers
	void register_w(int whichcpu, offs_t offset, uint8_t data);
	void bankswitch(int whichcpu, int whichbank, int a0, uint8_t data);
	void kick_watchdog(int whichcpu);

	// internal state
	uint32_t m_offsets[2][8];
	uint8_t m_subres, m_wdog;

	// callbacks
	devcb_write_line           m_subres_cb;

	// address space
	const address_space_config m_program_config;
	address_space *            m_program;

	// cpu interfaces
	required_device<cpu_device> m_cpuexec[2];
	memory_access_cache<0, 0, ENDIANNESS_BIG> *m_cpucache[2];

	required_device<watchdog_timer_device> m_watchdog;
};

// device type definition
DECLARE_DEVICE_TYPE(NAMCO_C117, namco_c117_device)

#endif // MAME_MACHINE_C117_H
