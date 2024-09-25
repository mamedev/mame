// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Gottlieb hardware

***************************************************************************/
#ifndef MAME_SHARED_GOTTLIEB_A_H
#define MAME_SHARED_GOTTLIEB_A_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m6503.h"
#include "machine/mos6530.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/okim6295.h"
#include "sound/sp0250.h"
#include "sound/votrax.h"
#include "sound/ymopm.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN2,         gottlieb_sound_p2_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN3,         gottlieb_sound_p3_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN4,         gottlieb_sound_p4_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN5,         gottlieb_sound_p5_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN6,         gottlieb_sound_p6_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN7,         gottlieb_sound_p7_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_REV1,         gottlieb_sound_r1_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_SPEECH_REV1,  gottlieb_sound_speech_r1_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_SPEECH_REV1A, gottlieb_sound_speech_r1a_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_REV2,         gottlieb_sound_r2_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gottlieb_sound_p2_device

class gottlieb_sound_p2_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_p2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// read/write
	void write(u8 data);

	// internal communications
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

protected:
	gottlieb_sound_p2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void write_sync(s32 param);
	void p2_map(address_map &map) ATTR_COLD;

	// devices
	required_device<m6503_device> m_cpu;
	required_device<mos6530_device> m_r6530;

private:
	u8 m_dummy; // needed for save-state support
};


// ======================> gottlieb_sound_p3_device

class gottlieb_sound_p3_device : public gottlieb_sound_p2_device
{
public:
	// construction/destruction
	gottlieb_sound_p3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void write_sync(s32 param) override;

private:
	void r6530b_w(u8 data);
	void p3_map(address_map &map) ATTR_COLD;

	bool m_irqclock;
	bool m_irqenable;
};


// ======================> gottlieb_sound_r1_device

// rev 1 sound board, with unpopulated SC-01[-A] and support circuitry
class gottlieb_sound_r1_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_r1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// read/write
	void write(u8 data);

protected:
	gottlieb_sound_r1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void write_sync(s32 param);

	virtual void r1_map(address_map &map) ATTR_COLD;

	// devices
	required_device<mc1408_device> m_dac;
	required_device<mos6532_device> m_riot;

private:
	u8 m_dummy; // needed for save-state support
};


// ======================> gottlieb_sound_speech_r1_device

// fully populated rev 1 sound board with SC-01 installed
class gottlieb_sound_speech_r1_device : public gottlieb_sound_r1_device
{
public:
	// construction/destruction
	gottlieb_sound_speech_r1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	gottlieb_sound_speech_r1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override { m_votrax->set_unscaled_clock(m_speech_clock); }

	virtual void r1_map(address_map &map) override ATTR_COLD;

	// internal communications
	u32 convert_speech_clock(u8 data);

	// devices
	required_device<votrax_sc01_device> m_votrax;

private:
	// internal communications
	void votrax_data_w(u8 data);
	void speech_clock_dac_w(u8 data);

	// internal state
	u32 m_speech_clock;
};

// fully populated rev 1 sound board with SC-01-A installed
class gottlieb_sound_speech_r1a_device : public gottlieb_sound_speech_r1_device
{
public:
	// construction/destruction
	gottlieb_sound_speech_r1a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> gottlieb_sound_p4_device

// fully populated pin 4 sound board
class gottlieb_sound_p4_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_p4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// read/write
	void write(u8 data);

protected:
	gottlieb_sound_p4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void write_sync(s32 param);
	TIMER_CALLBACK_MEMBER(set_nmi);
	TIMER_CALLBACK_MEMBER(clear_nmi);

	// internal communications
	u8 speech_data_r();
	u8 audio_data_r();
	u8 signal_audio_nmi_r();
	void signal_audio_nmi_w(u8 data = 0);
	void nmi_rate_w(u8 data);
	void speech_ctrl_w(u8 data);
	void psg_latch_w(u8 data);

	void p4_dmap(address_map &map) ATTR_COLD;
	void p4_ymap(address_map &map) ATTR_COLD;

	// internal helpers
	void nmi_timer_adjust();
	void nmi_state_update();

	// devices
	required_device<m6502_device> m_dcpu;
	optional_device<m6502_device> m_dcpu2;
	required_device<m6502_device> m_ycpu;
	required_device<ay8913_device> m_ay1;
	required_device<ay8913_device> m_ay2;

	// internal state
	emu_timer *m_nmi_timer;
	emu_timer *m_nmi_clear_timer;
	u8 m_nmi_rate;
	u8 m_nmi_state;
	u8 m_dcpu_latch;
	u8 m_ycpu_latch;
	u8 m_speech_control;
	u8 m_last_command;
	u8 m_psg_latch;
	u8 m_psg_data_latch;
	u8 m_dcpu2_latch;
};


// ======================> gottlieb_sound_r2_device

// fully populated rev 2 sound board
class gottlieb_sound_r2_device : public gottlieb_sound_p4_device
{
public:
	// construction/destruction
	gottlieb_sound_r2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	void enable_cobram3_mods() { m_cobram3_mod = true; }

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	// internal communications
	void sp0250_latch_w(u8 data);
	void speech_control_w(u8 data);

	void r2_dmap(address_map &map) ATTR_COLD;
	void r2_ymap(address_map &map) ATTR_COLD;

	// devices
	optional_device<sp0250_device> m_sp0250;

	// internal state
	bool m_cobram3_mod;
	u8 m_sp0250_latch;
};


// ======================> gottlieb_sound_p5_device

// same as p4 plus a YM2151 in the expansion socket
class gottlieb_sound_p5_device : public gottlieb_sound_p4_device
{
public:
	// construction/destruction
	gottlieb_sound_p5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	gottlieb_sound_p5_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void p5_ymap(address_map &map) ATTR_COLD;

	optional_device<ym2151_device> m_ym2151;
};


// ======================> gottlieb_sound_p6_device

// same as p5 plus an extra dac, same as existing audiocpu. For bonebusters.
class gottlieb_sound_p6_device : public gottlieb_sound_p5_device
{
public:
	// construction/destruction
	gottlieb_sound_p6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void p6_dmap(address_map &map) ATTR_COLD;
	u8 d2_data_r();
};


// ======================> gottlieb_sound_p7_device

// same as p5 plus MSM6295.
class gottlieb_sound_p7_device : public gottlieb_sound_p5_device
{
public:
	// construction/destruction
	gottlieb_sound_p7_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void p7_ymap(address_map &map) ATTR_COLD;
	void y_ctrl_w(u8);
	void y_latch_w(u8);

	optional_device<okim6295_device> m_oki;

	u8 m_msm_latch1;
	u8 m_msm_latch2;
};

#endif // MAME_SHARED_GOTTLIEB_A_H
