// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "cpu/m6502/m6502.h"
#include "machine/mos6530.h"
#include "machine/6532riot.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/okim6295.h"
#include "sound/sp0250.h"
#include "sound/votrax.h"
#include "sound/ymopm.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN2,        gottlieb_sound_p2_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN3,        gottlieb_sound_p3_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN4,        gottlieb_sound_p4_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN5,        gottlieb_sound_p5_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN6,        gottlieb_sound_p6_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN7,        gottlieb_sound_p7_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_REV1,        gottlieb_sound_r1_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_REV1_VOTRAX, gottlieb_sound_r1_with_votrax_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_REV2,        gottlieb_sound_r2_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gottlieb_sound_p2_device

class gottlieb_sound_p2_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_p2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// read/write
	void write(uint8_t data);

	// internal communications
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

	void p2_map(address_map &map);

private:
	// devices
	required_device<m6502_device>       m_cpu;
	required_device<mos6530_device>     m_r6530;

	uint8_t m_sndcmd = 0;

	uint8_t r6530b_r();
};


// ======================> gottlieb_sound_p3_device

class gottlieb_sound_p3_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_p3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// read/write
	void write(uint8_t data);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	void p3_map(address_map &map);

private:
	// devices
	required_device<m6502_device>       m_cpu;
	required_device<mos6530_device>     m_r6530;

	uint8_t m_sndcmd = 0;

	uint8_t r6530b_r();
	void r6530b_w(u8);
};


// ======================> gottlieb_sound_r1_device

// rev 1 sound board, with unpopulated VOTRAX
class gottlieb_sound_r1_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_r1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// read/write
	void write(u8 data);

protected:
	gottlieb_sound_r1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

	virtual void r1_map(address_map &map);

protected:
	required_device<mc1408_device> m_dac;

private:
	// devices
	required_device<riot6532_device> m_riot;
	u8 m_dummy = 0;   // needed for save-state support
};


// ======================> gottlieb_sound_r1_with_votrax_device

// fully populated rev 1 sound board
class gottlieb_sound_r1_with_votrax_device : public gottlieb_sound_r1_device
{
public:
	// construction/destruction
	gottlieb_sound_r1_with_votrax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_post_load() override;

	// internal communications
	void votrax_data_w(uint8_t data);
	void speech_clock_dac_w(uint8_t data);

	virtual void r1_map(address_map &map) override;

private:
	// devices
	required_device<votrax_sc01_device> m_votrax;

	// internal state
	uint8_t m_last_speech_clock = 0;
};


// ======================> gottlieb_sound_p4_device

// fully populated pin 4 sound board
class gottlieb_sound_p4_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_p4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// read/write
	void write(u8 data);

protected:
	gottlieb_sound_p4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	TIMER_CALLBACK_MEMBER(set_nmi);
	TIMER_CALLBACK_MEMBER(clear_nmi);
	TIMER_CALLBACK_MEMBER(update_latch);

	// internal communications
	uint8_t speech_data_r();
	uint8_t audio_data_r();
	uint8_t signal_audio_nmi_r();
	void signal_audio_nmi_w(uint8_t data);
	void nmi_rate_w(uint8_t data);
	void speech_ctrl_w(uint8_t data);
	void psg_latch_w(uint8_t data);

	void p4_dmap(address_map &map);
	void p4_ymap(address_map &map);

	// internal helpers
	void nmi_timer_adjust();
	void nmi_state_update();

	// devices
	required_device<m6502_device>   m_dcpu;
	optional_device<m6502_device>   m_dcpu2;
	required_device<m6502_device>   m_ycpu;
	required_device<ay8913_device>  m_ay1;
	required_device<ay8913_device>  m_ay2;

	// internal state
	emu_timer *   m_nmi_timer;
	emu_timer *   m_nmi_clear_timer;
	emu_timer *   m_latch_timer;
	uint8_t       m_nmi_rate;
	uint8_t       m_nmi_state;
	uint8_t       m_dcpu_latch;
	uint8_t       m_ycpu_latch;
	uint8_t       m_speech_control;
	uint8_t       m_last_command;
	uint8_t       m_psg_latch;
	uint8_t       m_psg_data_latch;
	uint8_t       m_dcpu2_latch;
};


// ======================> gottlieb_sound_r2_device

// fully populated rev 2 sound board
class gottlieb_sound_r2_device : public gottlieb_sound_p4_device
{
public:
	// construction/destruction
	gottlieb_sound_r2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration helpers
	void enable_cobram3_mods() { m_cobram3_mod = true; }

	CUSTOM_INPUT_MEMBER( speech_drq_custom_r );

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

private:
	// internal communications
	void sp0250_latch_w(uint8_t data);
	void speech_control_w(uint8_t data);

	void r2_dmap(address_map &map);
	void r2_ymap(address_map &map);

	// devices
	optional_device<sp0250_device>  m_sp0250;

	// internal state
	bool     m_cobram3_mod = 0;
	uint8_t  m_sp0250_latch = 0;
};


// ======================> gottlieb_sound_p5_device

// same as p5 plus a YM2151 in the expansion socket
class gottlieb_sound_p5_device : public gottlieb_sound_p4_device
{
public:
	// construction/destruction
	gottlieb_sound_p5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	gottlieb_sound_p5_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	void p5_ymap(address_map &map);
	optional_device<ym2151_device> m_ym2151;
};


// ======================> gottlieb_sound_p6_device

// same as p5 plus an extra dac, same as existing audiocpu. For bonebusters.
class gottlieb_sound_p6_device : public gottlieb_sound_p5_device
{
public:
	// construction/destruction
	gottlieb_sound_p6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	void p6_dmap(address_map &map);
	uint8_t d2_data_r();
};


// ======================> gottlieb_sound_p7_device

// same as p5 plus MSM6295.
class gottlieb_sound_p7_device : public gottlieb_sound_p5_device
{
public:
	// construction/destruction
	gottlieb_sound_p7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	void p7_ymap(address_map &map);
	void y_ctrl_w(u8);
	void y_latch_w(u8);
	uint8_t m_msm_latch1 = 0;
	uint8_t m_msm_latch2 = 0;
	optional_device<okim6295_device> m_oki;
};

