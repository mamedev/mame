// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/mos6530.h"
#include "machine/6532riot.h"
#include "sound/ay8910.h"
#include "sound/sp0250.h"
#include "sound/votrax.h"


// set to 0 to enable Votrax device and disable samples
#define USE_FAKE_VOTRAX         (1)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const device_type GOTTLIEB_SOUND_REV0;
extern const device_type GOTTLIEB_SOUND_REV1;
extern const device_type GOTTLIEB_SOUND_REV1_WITH_VOTRAX;
extern const device_type GOTTLIEB_SOUND_REV2;



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GOTTLIEB_ENABLE_COBRAM3_MODS() \
	gottlieb_sound_r2_device::static_enable_cobram3_mods(*device);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gottlieb_sound_r0_device

// rev 0 sound board
class gottlieb_sound_r0_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_r0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// internal communications
	uint8_t r6530b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void audio_nmi(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

private:
	// devices
	required_device<m6502_device>       m_audiocpu;
	required_device<mos6530_device>     m_r6530;

	uint8_t m_sndcmd;
};

// ======================> gottlieb_sound_r1_device

// rev 1 sound board, with unpopulated VOTRAX
class gottlieb_sound_r1_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_r1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	gottlieb_sound_r1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, bool populate_votrax);

	// read/write
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// internal communications
	void snd_interrupt(int state);
	void r6532_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void votrax_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void speech_clock_dac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void votrax_request(int state);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

private:
	// devices
	required_device<m6502_device>       m_audiocpu;
	required_device<riot6532_device>    m_riot;
	optional_device<votrax_sc01_device> m_votrax;

	// internal state
	//bool            m_populate_votrax;
	uint8_t           m_last_speech_clock;

#if USE_FAKE_VOTRAX
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
private:
	void fake_votrax_data_w(uint8_t data);
	void trigger_sample(uint8_t data);
	optional_device<samples_device> m_samples;
	uint8_t m_score_sample;
	uint8_t m_random_offset;
	uint8_t m_votrax_queue[100];
	uint8_t m_votrax_queuepos;
#endif
};

// fully populated rev 1 sound board
class gottlieb_sound_r1_with_votrax_device : public gottlieb_sound_r1_device
{
public:
	// construction/destruction
	gottlieb_sound_r1_with_votrax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
};


// ======================> gottlieb_sound_r2_device

// fully populated rev 2 sound board
class gottlieb_sound_r2_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_r2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	static void static_enable_cobram3_mods(device_t &device);

	// read/write
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// internal communications
	uint8_t speech_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t audio_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void signal_audio_nmi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmi_rate_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value speech_drq_custom_r(ioport_field &field, void *param);
	void speech_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sp0250_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void psg_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

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


/*----------- defined in audio/gottlieb.c -----------*/

#if USE_FAKE_VOTRAX
MACHINE_CONFIG_EXTERN( reactor_samples );
MACHINE_CONFIG_EXTERN( qbert_samples );
#endif
