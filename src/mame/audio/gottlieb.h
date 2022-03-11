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
#include "sound/sp0250.h"
#include "sound/votrax.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN2,        gottlieb_sound_p2_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_REV1,        gottlieb_sound_r1_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_REV1_VOTRAX, gottlieb_sound_r1_with_votrax_device)
DECLARE_DEVICE_TYPE(GOTTLIEB_SOUND_REV2,        gottlieb_sound_r2_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gottlieb_sound_p2_device

// rev 0 sound board
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

	void gottlieb_sound_p2_map(address_map &map);

private:
	// devices
	required_device<m6502_device>       m_audiocpu;
	required_device<mos6530_device>     m_r6530;

	uint8_t m_sndcmd;

	uint8_t r6530b_r();
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
	gottlieb_sound_r1_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

	virtual void gottlieb_sound_r1_map(address_map &map);

protected:
	required_device<dac_8bit_r2r_device> m_dac;

private:
	// devices
	required_device<riot6532_device> m_riot;
	u8 m_dummy = 0;   // needed for save-state support
};

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

	virtual void gottlieb_sound_r1_map(address_map &map) override;

private:
	// devices
	required_device<votrax_sc01_device> m_votrax;

	// internal state
	uint8_t m_last_speech_clock = 0;
};


// ======================> gottlieb_sound_r2_device

// fully populated rev 2 sound board
class gottlieb_sound_r2_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_r2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration helpers
	void enable_cobram3_mods() { m_cobram3_mod = true; }

	// read/write
	void write(u8 data);

	// internal communications
	uint8_t speech_data_r();
	uint8_t audio_data_r();
	uint8_t signal_audio_nmi_r();
	void signal_audio_nmi_w(uint8_t data);
	void nmi_rate_w(uint8_t data);
	CUSTOM_INPUT_MEMBER( speech_drq_custom_r );
	void speech_control_w(uint8_t data);
	void sp0250_latch_w(uint8_t data);
	void psg_latch_w(uint8_t data);

	void gottlieb_sound_r2_map(address_map &map);
	void gottlieb_speech_r2_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	// internal helpers
	void nmi_timer_adjust();
	void nmi_state_update();

	// timer IDs
	enum
	{
		TID_NMI_GENERATE,
		TID_NMI_CLEAR,
		TID_SOUND_LATCH_WRITE
	};

	// devices
	required_device<m6502_device>   m_audiocpu;
	required_device<m6502_device>   m_speechcpu;
	required_device<ay8913_device>  m_ay1;
	required_device<ay8913_device>  m_ay2;
	optional_device<sp0250_device>  m_sp0250;

	// internal state
	bool        m_cobram3_mod;
	emu_timer * m_nmi_timer;
	uint8_t       m_nmi_rate;
	uint8_t       m_nmi_state;
	uint8_t       m_audiocpu_latch;
	uint8_t       m_speechcpu_latch;
	uint8_t       m_speech_control;
	uint8_t       m_last_command;
	uint8_t       m_psg_latch;
	uint8_t       m_psg_data_latch;
	uint8_t       m_sp0250_latch;
};

