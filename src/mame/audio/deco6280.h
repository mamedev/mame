// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    deco6280.h

    Functions to emulate the Data East HuC6280( Sticker says "45" at most Data East PCBs ) CPU Based Sound HWs

***************************************************************************/

#ifndef MAME_AUDIO_DECO6280_H
#define MAME_AUDIO_DECO6280_H

#pragma once

#include "cpu/h6280/h6280.h"
#include "sound/ym2203.h"
#include "sound/ym2151.h"
#include "sound/okim6295.h"

#define DECO_YM2151_OUT0    0
#define DECO_YM2151_OUT1    1
#define DECO_OKI1_OUT       2
#define DECO_OKI2_OUT       3
#define DECO_YM2203_OUT0    4
#define DECO_YM2203_OUT1    5
#define DECO_YM2203_OUT2    6
#define DECO_YM2203_OUT3    7

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(DECO6280,              deco_6280_base_device)
DECLARE_DEVICE_TYPE(DECO6280_2XOKI,        deco_6280_2xoki_device)
DECLARE_DEVICE_TYPE(DECO6280_YM2203_2XOKI, deco_6280_ym2203_2xoki_device)



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DECO6280_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, DECO6280, _clock) \

#define MCFG_DECO6280_2XOKI_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, DECO6280_2XOKI, _clock) \

#define MCFG_DECO6280_2XOKI_YM2203_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, DECO6280_YM2203_2XOKI, _clock) \

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> deco_6280_base_device

class deco_6280_base_device :   public device_t,
								public device_mixer_interface
{
protected:
	// derived construction/destruction
	deco_6280_base_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner, uint32_t clock, int channels);

public:
	// construction/destruction
	deco_6280_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration
	template <class Object> static devcb_base &static_set_soundlatch_cb(device_t &device, Object &&cb) { return downcast<deco_6280_base_device &>(device).m_soundlatch_cb.set_callback(std::forward<Object>(cb)); }

	// getters
	h6280_device &soundcpu() const { return *m_cpu; }

	// read/write handlers
	DECLARE_READ8_MEMBER(soundlatch_r);
	DECLARE_WRITE8_MEMBER(soundlatch_w);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// devices
	required_device<h6280_device> m_cpu;
	required_device<ym2151_device> m_ym2151;
	required_device<okim6295_device> m_oki1;
	
	// memory regions
	required_memory_region m_oki1_region;

	// configuration state
	devcb_read8    m_soundlatch_cb;

	// internal state
	uint8_t m_soundlatch;
};


// ======================> deco_6280_2xoki_device

class deco_6280_2xoki_device : public deco_6280_base_device
{
protected:
	// derived construction/destruction
	deco_6280_2xoki_device(const machine_config &mconfig, device_type devtype, const char *tag, device_t *owner, uint32_t clock, int channels);

public:
	// construction/destruction
	deco_6280_2xoki_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration
	template <class Object> static devcb_base &static_set_ym2151_cb(device_t &device, Object &&cb) { return downcast<deco_6280_2xoki_device &>(device).m_ym2151_cb.set_callback(std::forward<Object>(cb)); }

	// read/write handlers
	DECLARE_WRITE8_MEMBER(ym2151_port_w);
	
	void oki1_bank_w(uint32_t bank, uint32_t mask);
	void oki2_bank_w(uint32_t bank, uint32_t mask);
protected:
	// device level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	// devices
	required_device<okim6295_device> m_oki2;

	// memory regions
	required_memory_region m_oki2_region;

	// memory banks
	required_memory_bank m_oki1_bank;
	required_memory_bank m_oki2_bank;
	
	// configuration state
	devcb_write8   m_ym2151_cb;

	// internal state
	uint32_t m_oki1_bankbase;
	uint32_t m_oki2_bankbase;
};


// ======================> deco_6280_ym2203_2xoki_device

class deco_6280_ym2203_2xoki_device : public deco_6280_2xoki_device
{
public:
	// derived construction/destruction
	deco_6280_ym2203_2xoki_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	// devices
	required_device<ym2203_device> m_ym2203;
};

#endif // MAME_AUDIO_DECO6280_H
