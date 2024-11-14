// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    williams.h

    Functions to emulate general the various Williams/Midway sound cards.

****************************************************************************/
#ifndef MAME_SHARED_WILLIAMSSOUND_H
#define MAME_SHARED_WILLIAMSSOUND_H

#pragma once

#include "machine/6821pia.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "sound/hc55516.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(WILLIAMS_CVSD_SOUND, williams_cvsd_sound_device)
DECLARE_DEVICE_TYPE(WILLIAMS_NARC_SOUND, williams_narc_sound_device)
DECLARE_DEVICE_TYPE(WILLIAMS_ADPCM_SOUND, williams_adpcm_sound_device)
DECLARE_DEVICE_TYPE(WILLIAMS_S4_SOUND, williams_s4_sound_device)
DECLARE_DEVICE_TYPE(WILLIAMS_S6_SOUND, williams_s6_sound_device)
DECLARE_DEVICE_TYPE(WILLIAMS_S9_SOUND, williams_s9_sound_device)
DECLARE_DEVICE_TYPE(WILLIAMS_S11_SOUND, williams_s11_sound_device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> williams_cvsd_sound_device

class williams_cvsd_sound_device :  public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	williams_cvsd_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// read/write
	void write(u16 data);
	void reset_write(int state);

	// internal communications
	void bank_select_w(u8 data);
	void cvsd_digit_clock_clear_w(u8 data);
	void cvsd_clock_set_w(u8 data);

	void williams_cvsd_map(address_map &map) ATTR_COLD;

	mc6809e_device *get_cpu() { return m_cpu; }

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sync_write);

private:
	// devices
	required_device<mc6809e_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<ym2151_device> m_ym2151;
	required_device<hc55516_device> m_hc55516;

	required_memory_bank m_rombank;

	// internal state
	emu_timer *m_sync_write_timer;
	u8 m_talkback;

	void talkback_w(u8 data);
};


// ======================> williams_narc_sound_device

class williams_narc_sound_device :  public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	williams_narc_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// read/write
	u16 read();
	void write(u16 data);
	void reset_write(int state);

	// internal communications
	void master_bank_select_w(u8 data);
	void slave_bank_select_w(u8 data);
	u8 command_r();
	void command2_w(u8 data);
	u8 command2_r();
	void master_talkback_w(u8 data);
	void master_sync_w(u8 data);
	void slave_talkback_w(u8 data);
	void slave_sync_w(u8 data);
	void cvsd_digit_clock_clear_w(u8 data);
	void cvsd_clock_set_w(u8 data);

	void williams_narc_master_map(address_map &map) ATTR_COLD;
	void williams_narc_slave_map(address_map &map) ATTR_COLD;

	mc6809e_device *get_cpu() { return m_cpu[0]; }

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sync_master_command);
	TIMER_CALLBACK_MEMBER(sync_slave_command);
	TIMER_CALLBACK_MEMBER(sync_clear);

private:
	// devices
	required_device_array<mc6809e_device, 2> m_cpu;
	required_device<hc55516_device> m_hc55516;

	required_memory_bank m_masterbank;
	required_memory_bank m_slavebank;

	// internal state
	emu_timer *m_sync_master_timer;
	emu_timer *m_sync_slave_timer;
	emu_timer *m_sync_clear_timer;
	u8 m_latch;
	u8 m_latch2;
	u8 m_talkback;
	u8 m_audio_sync;
	u8 m_sound_int_state;
};


// ======================> williams_adpcm_sound_device

class williams_adpcm_sound_device : public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	williams_adpcm_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// read/write
	void write(u16 data);
	void reset_write(int state);
	int irq_read();

	// internal communications
	void bank_select_w(u8 data);
	void oki6295_bank_select_w(u8 data);
	u8 command_r();
	void talkback_w(u8 data);

	void williams_adpcm_map(address_map &map) ATTR_COLD;
	void williams_adpcm_oki_map(address_map &map) ATTR_COLD;

	mc6809e_device *get_cpu() { return m_cpu; }

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sync_command);
	TIMER_CALLBACK_MEMBER(irq_clear);

private:
	// devices
	required_device<mc6809e_device> m_cpu;

	required_memory_bank m_rombank;
	required_memory_bank m_okibank;

	// internal state
	emu_timer *m_sync_command_timer;
	emu_timer *m_irq_clear_timer;
	u8 m_latch;
	u8 m_talkback;
	u8 m_sound_int_state;
};


// ======================> williams_s4_sound_device

class williams_s4_sound_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	williams_s4_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// read/write
	void write(u8 data);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

protected:

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// devices
	void williams_s4_map(address_map &map) ATTR_COLD;
	required_device<m6808_cpu_device> m_cpu;
	required_device<pia6821_device> m_pia;
	u8 m_dummy = 0;   // needed for save-state support
};


// ======================> williams_s6_sound_device

class williams_s6_sound_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	williams_s6_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// read/write
	void write(u8 data);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

protected:

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void pb_w(u8 data);
	// devices
	void williams_s6_map(address_map &map) ATTR_COLD;
	required_device<m6802_cpu_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<hc55516_device> m_hc;
	u8 m_dummy = 0;   // needed for save-state support
};


// ======================> williams_s9_sound_device

class williams_s9_sound_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	williams_s9_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// read/write
	void write(u8 data);
	void strobe(int state);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

protected:

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// devices
	void williams_s9_map(address_map &map) ATTR_COLD;
	required_device<m6802_cpu_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<hc55516_device> m_hc;
	u8 m_dummy = 0;   // needed for save-state support
};


// ======================> williams_s11_sound_device

class williams_s11_sound_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	williams_s11_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// read/write
	void write(u8 data);
	void strobe(int state);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

protected:

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void bank_w(u8);
	// devices
	void williams_s11_map(address_map &map) ATTR_COLD;
	required_device<m6802_cpu_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<hc55516_device> m_hc;
	u8 m_dummy = 0;   // needed for save-state support
};

#endif // MAME_SHARED_WILLIAMSSOUND_H
