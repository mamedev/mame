// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    bsmt2000.h

    BSMT2000 device emulator.

***************************************************************************/

#pragma once

#ifndef __BSMT2000_H__
#define __BSMT2000_H__

#include "cpu/tms32010/tms32010.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BSMT2000_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, BSMT2000, _clock)
#define MCFG_BSMT2000_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, BSMT2000, _clock)
#define MCFG_BSMT2000_READY_CALLBACK(_callback) \
	bsmt2000_device::static_set_ready_callback(*device, _callback);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> bsmt2000_device

class bsmt2000_device : public device_t,
						public device_sound_interface,
						public device_memory_interface
{
	typedef void (*ready_callback)(bsmt2000_device &device);

public:
	// construction/destruction
	bsmt2000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_ready_callback(device_t &device, ready_callback callback);

	// public interface
	UINT16 read_status();
	void write_reg(UINT16 data);
	void write_data(UINT16 data);

protected:
	// device-level overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	// internal TMS I/O callbacks
	DECLARE_READ16_MEMBER( tms_register_r );
	DECLARE_READ16_MEMBER( tms_data_r );
	DECLARE_READ16_MEMBER( tms_rom_r );
	DECLARE_WRITE16_MEMBER( tms_rom_addr_w );
	DECLARE_WRITE16_MEMBER( tms_rom_bank_w );
	DECLARE_WRITE16_MEMBER( tms_left_w );
	DECLARE_WRITE16_MEMBER( tms_right_w );
	DECLARE_READ16_MEMBER( tms_write_pending_r );

private:
	// timers
	enum
	{
		TIMER_ID_RESET,
		TIMER_ID_REG_WRITE,
		TIMER_ID_DATA_WRITE
	};

	// configuration state
	const address_space_config  m_space_config;
	ready_callback              m_ready_callback;

	// internal state
	sound_stream *              m_stream;
	direct_read_data *          m_direct;
	tms32015_device *           m_cpu;
	UINT16                      m_register_select;
	UINT16                      m_write_data;
	UINT16                      m_rom_address;
	UINT16                      m_rom_bank;
	INT16                       m_left_data;
	INT16                       m_right_data;
	bool                        m_write_pending;
};


// device type definition
extern const device_type BSMT2000;


#endif /* __BSMT2000_H__ */
