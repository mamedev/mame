// license:BSD-3-Clause
// copyright-holders:Alex W. Jackson

#pragma once

#ifndef __C117_H__
#define __C117_H__

#include "machine/watchdog.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CUS117_CPUS(_maincpu, _subcpu) \
		namco_c117_device::set_cpu_tags(*device, _maincpu, _subcpu);

#define MCFG_CUS117_SUBRES_CB(_devcb) \
		devcb = &namco_c117_device::set_subres_cb(*device, DEVCB_##_devcb);


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

	// static configuration
	static void set_cpu_tags(device_t &device, const char *maintag, const char *subtag);
	template<class _Object> static devcb_base &set_subres_cb(device_t &device, _Object object) { return downcast<namco_c117_device &>(device).m_subres_cb.set_callback(object); }

	uint8_t main_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sub_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void main_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sub_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// FIXME: this doesn't belong here
	void sound_watchdog_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	offs_t remap(int whichcpu, offs_t offset) { return m_offsets[whichcpu][offset>>13] | (offset & 0x1fff); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

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
	device_execute_interface * m_cpuexec[2];
	direct_read_data *         m_cpudirect[2];

	// configuration
	const char *               m_maincpu_tag;
	const char *               m_subcpu_tag;

	required_device<watchdog_timer_device> m_watchdog;
};

// device type definition
extern const device_type NAMCO_C117;

#endif
