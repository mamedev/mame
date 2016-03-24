// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarijsa.h

    Functions to emulate the Atari "JSA" audio boards

***************************************************************************/

#pragma once

#ifndef __ATARI_JSA__
#define __ATARI_JSA__

#include "cpu/m6502/m6502.h"
#include "sound/tms5220.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "sound/pokey.h"
#include "machine/atarigen.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const device_type ATARI_JSA_I;
extern const device_type ATARI_JSA_II;
extern const device_type ATARI_JSA_III;
extern const device_type ATARI_JSA_IIIS;



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ATARI_JSA_I_ADD(_tag, _intcb) \
	MCFG_DEVICE_ADD(_tag, ATARI_JSA_I, 0) \
	devcb = &atari_jsa_i_device::static_set_main_int_cb(*device, DEVCB_##_intcb);

#define MCFG_ATARI_JSA_II_ADD(_tag, _intcb) \
	MCFG_DEVICE_ADD(_tag, ATARI_JSA_II, 0) \
	devcb = &atari_jsa_ii_device::static_set_main_int_cb(*device, DEVCB_##_intcb);

#define MCFG_ATARI_JSA_III_ADD(_tag, _intcb) \
	MCFG_DEVICE_ADD(_tag, ATARI_JSA_III, 0) \
	devcb = &atari_jsa_iii_device::static_set_main_int_cb(*device, DEVCB_##_intcb);

#define MCFG_ATARI_JSA_IIIS_ADD(_tag, _intcb) \
	MCFG_DEVICE_ADD(_tag, ATARI_JSA_IIIS, 0) \
	devcb = &atari_jsa_iiis_device::static_set_main_int_cb(*device, DEVCB_##_intcb);

#define MCFG_ATARI_JSA_TEST_PORT(_port, _bitnum) \
	devcb = &atari_jsa_base_device::static_set_test_read_cb(*device, DEVCB_IOPORT(_port)); \
	MCFG_DEVCB_RSHIFT(_bitnum);


//**************************************************************************
//  I/O PORT BIT HELPERS
//**************************************************************************

#define PORT_ATARI_JSA_SOUND_TO_MAIN_READY(_tag) \
	PORT_READ_LINE_DEVICE_MEMBER(_tag, atari_jsa_base_device, sound_to_main_ready)

#define PORT_ATARI_JSA_MAIN_TO_SOUND_READY(_tag) \
	PORT_READ_LINE_DEVICE_MEMBER(_tag, atari_jsa_base_device, main_to_sound_ready)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> atari_jsa_base_device

class atari_jsa_base_device :   public device_t,
								public device_mixer_interface
{
protected:
	// construction/destruction
	atari_jsa_base_device(const machine_config &mconfig, device_type devtype, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int channels);

public:
	// static configuration
	template<class _Object> static devcb_base &static_set_test_read_cb(device_t &device, _Object object) { return downcast<atari_jsa_base_device &>(device).m_test_read_cb.set_callback(object); }
	template<class _Object> static devcb_base &static_set_main_int_cb(device_t &device, _Object object) { return downcast<atari_jsa_base_device &>(device).m_main_int_cb.set_callback(object); }

	// getters
	m6502_device &soundcpu() const { return *m_jsacpu; }
	DECLARE_READ_LINE_MEMBER(main_to_sound_ready) { return m_soundcomm->main_to_sound_ready(); }
	DECLARE_READ_LINE_MEMBER(sound_to_main_ready) { return m_soundcomm->sound_to_main_ready(); }

	// main cpu accessors
	DECLARE_WRITE8_MEMBER(main_command_w);
	DECLARE_READ8_MEMBER(main_response_r);
	DECLARE_WRITE16_MEMBER(sound_reset_w);

	// read/write handlers
	DECLARE_WRITE8_MEMBER(ym2151_port_w);
	DECLARE_READ_LINE_MEMBER(main_test_read_line);

	// I/O lines
	DECLARE_WRITE_LINE_MEMBER(main_int_write_line);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal helpers
	virtual void update_all_volumes() = 0;

	// devices
	required_device<atari_sound_comm_device> m_soundcomm;
	required_device<m6502_device> m_jsacpu;
	required_device<ym2151_device> m_ym2151;

	// memory banks
	required_memory_bank m_cpu_bank;

	// configuration state
	devcb_read_line    m_test_read_cb;
	devcb_write_line   m_main_int_cb;

	// internal state
	double              m_ym2151_volume;
	UINT8               m_ym2151_ct1;
	UINT8               m_ym2151_ct2;
};


// ======================> atari_jsa_oki_base_device

class atari_jsa_oki_base_device : public atari_jsa_base_device
{
protected:
	// derived construction/destruction
	atari_jsa_oki_base_device(const machine_config &mconfig, device_type devtype, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int channels);

public:
	// read/write handlers
	DECLARE_READ8_MEMBER( oki_r );
	DECLARE_WRITE8_MEMBER( oki_w );
	DECLARE_WRITE8_MEMBER( wrio_w );
	DECLARE_WRITE8_MEMBER( mix_w );
	DECLARE_WRITE8_MEMBER( overall_volume_w );

protected:
	// device level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal helpers
	virtual void update_all_volumes() override;

	// devices
	optional_device<okim6295_device> m_oki1;
	optional_device<okim6295_device> m_oki2;    // JSA IIIs only

	// memory banks
	optional_memory_bank m_oki1_banklo;         // JSA III(s) only
	optional_memory_bank m_oki1_bankhi;         // JSA III(s)
	optional_memory_bank m_oki2_banklo;         // JSA IIIs only
	optional_memory_bank m_oki2_bankhi;         // JSA IIIs only

	// internal state
	double              m_oki6295_volume;
	double              m_overall_volume;       // JSA III(s) only
};


// ======================> atari_jsa_i_device

class atari_jsa_i_device : public atari_jsa_base_device
{
public:
	// construction/destruction
	atari_jsa_i_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// read/write handlers
	DECLARE_READ8_MEMBER( rdio_r );
	DECLARE_WRITE8_MEMBER( wrio_w );
	DECLARE_WRITE8_MEMBER( mix_w );
	DECLARE_WRITE8_MEMBER( tms5220_voice );
	DECLARE_READ8_MEMBER( pokey_r );
	DECLARE_WRITE8_MEMBER( pokey_w );

protected:
	// device level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal helpers
	virtual void update_all_volumes() override;

	// devices
	optional_device<pokey_device> m_pokey;
	optional_device<tms5220_device> m_tms5220;
	required_ioport m_jsai;

	// internal state
	double              m_pokey_volume;
	double              m_tms5220_volume;
};


// ======================> atari_jsa_ii_device

class atari_jsa_ii_device : public atari_jsa_oki_base_device
{
public:
	// construction/destruction
	atari_jsa_ii_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// read/write handlers
	DECLARE_READ8_MEMBER( rdio_r );

protected:
	// device level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	required_ioport m_jsaii;
};


// ======================> atari_jsa_iii_device

class atari_jsa_iii_device : public atari_jsa_oki_base_device
{
public:
	// construction/destruction
	atari_jsa_iii_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// derived construction/destruction
	atari_jsa_iii_device(const machine_config &mconfig, device_type devtype, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int channels);

public:
	// read/write handlers
	DECLARE_READ8_MEMBER( rdio_r );

protected:
	// device level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	required_ioport m_jsaiii;
};


// ======================> atari_jsa_iiis_device

class atari_jsa_iiis_device : public atari_jsa_iii_device
{
public:
	// construction/destruction
	atari_jsa_iiis_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};


#endif
