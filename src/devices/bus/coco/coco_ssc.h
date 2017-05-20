// license:BSD-3-Clause
// copyright-holders:tim lindner
#pragma once

#ifndef MAME_DEVICES_BUS_COCO_COCO_SSC_H
#define MAME_DEVICES_BUS_COCO_COCO_SSC_H

#include "machine/ram.h"
#include "sound/ay8910.h"
#include "sound/sp0256.h"
#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"
#include "cococart.h"

// #define SAC_NETLIST_ON

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cocossc_sac_device;

// ======================> coco_ssc_device

class coco_ssc_device :
		public device_t,
		public device_cococart_interface
{
public:
		// construction/destruction
		coco_ssc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override;
		virtual machine_config_constructor device_mconfig_additions() const override;

		virtual void device_reset() override;

		DECLARE_READ8_MEMBER(ssc_port_a_r);
		DECLARE_WRITE8_MEMBER(ssc_port_b_w);
		DECLARE_READ8_MEMBER(ssc_port_c_r);
		DECLARE_WRITE8_MEMBER(ssc_port_c_w);
		DECLARE_READ8_MEMBER(ssc_port_d_r);
		DECLARE_WRITE8_MEMBER(ssc_port_d_w);

protected:
		// device-level overrides
		virtual void device_start() override;
		virtual DECLARE_READ8_MEMBER(ff7d_read);
		virtual DECLARE_WRITE8_MEMBER(ff7d_write);
		virtual void set_sound_enable(bool sound_enable) override;
private:
		uint8_t reset_line;
		uint8_t tms7000_porta;
		uint8_t tms7000_portb;
		uint8_t tms7000_portc;
		uint8_t tms7000_portd;
		required_device<cpu_device> m_tms7040;
		required_device<ram_device> m_staticram;
		required_device<ay8910_device> m_ay;
		required_device<sp0256_device> m_spo;
		required_device<cocossc_sac_device> m_sac;
};


// device type definition
extern const device_type COCO_SSC;


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_COCOSSC_SAC_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, COCOSSC_SAC, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> Color Computer Sound Activity Circuit filter

class cocossc_sac_device : public device_t,
								public device_sound_interface
{
public:
	cocossc_sac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~cocossc_sac_device() { }
	bool sound_activity_circuit_output();

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	sound_stream*  m_stream;
	double m_rms[16];
	int m_index;
};

extern const device_type COCOSSC_SAC;

#endif  // MAME_DEVICES_BUS_COCO_COCO_SSC_H
