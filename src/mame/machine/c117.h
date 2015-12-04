// license:BSD-3-Clause
// copyright-holders:Alex W. Jackson

#pragma once

#ifndef __C117_H__
#define __C117_H__


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
	namco_c117_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void set_cpu_tags(device_t &device, const char *maintag, const char *subtag);
	template<class _Object> static devcb_base &set_subres_cb(device_t &device, _Object object) { return downcast<namco_c117_device &>(device).m_subres_cb.set_callback(object); }

	DECLARE_READ8_MEMBER(main_r);
	DECLARE_READ8_MEMBER(sub_r);
	DECLARE_WRITE8_MEMBER(main_w);
	DECLARE_WRITE8_MEMBER(sub_w);

	// FIXME: this doesn't belong here
	DECLARE_WRITE8_MEMBER(sound_watchdog_w);

	offs_t remap(int whichcpu, offs_t offset) { return m_offsets[whichcpu][offset>>13] | (offset & 0x1fff); }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

private:
	// internal helpers
	void register_w(int whichcpu, offs_t offset, UINT8 data);
	void bankswitch(int whichcpu, int whichbank, int a0, UINT8 data);
	void kick_watchdog(int whichcpu);

	// internal state
	UINT32 m_offsets[2][8];
	UINT8 m_subres, m_wdog;

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
};

// device type definition
extern const device_type NAMCO_C117;

#endif
