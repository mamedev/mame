// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/mos6530.h"
#include "machine/6532riot.h"
#include "sound/dac.h"
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

#define MCFG_GOTTLIEB_SOUND_R0_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GOTTLIEB_SOUND_REV0, 0)

#define MCFG_GOTTLIEB_SOUND_R1_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GOTTLIEB_SOUND_REV1, 0)
#define MCFG_GOTTLIEB_SOUND_R1_ADD_VOTRAX(_tag) \
	MCFG_DEVICE_ADD(_tag, GOTTLIEB_SOUND_REV1_WITH_VOTRAX, 0)

#define MCFG_GOTTLIEB_SOUND_R2_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GOTTLIEB_SOUND_REV2, 0)
#define MCFG_GOTTLIEB_SOUND_R2_ADD_COBRAM3(_tag) \
	MCFG_DEVICE_ADD(_tag, GOTTLIEB_SOUND_REV2, 0) \
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
	gottlieb_sound_r0_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// read/write
	DECLARE_WRITE8_MEMBER( write );

	// internal communications
	DECLARE_READ8_MEMBER( r6530b_r );
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

private:
	// devices
	required_device<m6502_device>       m_audiocpu;
	required_device<mos6530_device>     m_r6530;
	required_device<dac_device>         m_dac;

	UINT8 m_sndcmd;
};

// ======================> gottlieb_sound_r1_device

// rev 1 sound board, with unpopulated VOTRAX
class gottlieb_sound_r1_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_r1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	gottlieb_sound_r1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock, bool populate_votrax);

	// read/write
	DECLARE_WRITE8_MEMBER( write );

	// internal communications
	DECLARE_WRITE_LINE_MEMBER( snd_interrupt );
	DECLARE_WRITE8_MEMBER( r6532_portb_w );
	DECLARE_WRITE8_MEMBER( votrax_data_w );
	DECLARE_WRITE8_MEMBER( speech_clock_dac_w );
	DECLARE_WRITE_LINE_MEMBER( votrax_request );

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

private:
	// devices
	required_device<m6502_device>       m_audiocpu;
	required_device<riot6532_device>    m_riot;
	required_device<dac_device>         m_dac;
	optional_device<votrax_sc01_device> m_votrax;

	// internal state
	//bool            m_populate_votrax;
	UINT8           m_last_speech_clock;

#if USE_FAKE_VOTRAX
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
private:
	void fake_votrax_data_w(UINT8 data);
	void trigger_sample(UINT8 data);
	optional_device<samples_device> m_samples;
	UINT8 m_score_sample;
	UINT8 m_random_offset;
	UINT8 m_votrax_queue[100];
	UINT8 m_votrax_queuepos;
#endif
};

// fully populated rev 1 sound board
class gottlieb_sound_r1_with_votrax_device : public gottlieb_sound_r1_device
{
public:
	// construction/destruction
	gottlieb_sound_r1_with_votrax_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

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
	gottlieb_sound_r2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_enable_cobram3_mods(device_t &device);

	// read/write
	DECLARE_WRITE8_MEMBER( write );

	// internal communications
	DECLARE_READ8_MEMBER( speech_data_r );
	DECLARE_READ8_MEMBER( audio_data_r );
	DECLARE_WRITE8_MEMBER( signal_audio_nmi_w );
	DECLARE_WRITE8_MEMBER( nmi_rate_w );
	CUSTOM_INPUT_MEMBER( speech_drq_custom_r );
	DECLARE_WRITE8_MEMBER( dac_w );
	DECLARE_WRITE8_MEMBER( speech_control_w );
	DECLARE_WRITE8_MEMBER( sp0250_latch_w );
	DECLARE_WRITE8_MEMBER( psg_latch_w );

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
	required_device<dac_device>     m_dac;
	required_device<ay8913_device>  m_ay1;
	required_device<ay8913_device>  m_ay2;
	optional_device<sp0250_device>  m_sp0250;

	// internal state
	bool        m_cobram3_mod;
	emu_timer * m_nmi_timer;
	UINT8       m_nmi_rate;
	UINT8       m_nmi_state;
	UINT8       m_audiocpu_latch;
	UINT8       m_speechcpu_latch;
	UINT8       m_speech_control;
	UINT8       m_last_command;
	UINT8       m_dac_data[2];
	UINT8       m_psg_latch;
	UINT8       m_psg_data_latch;
	UINT8       m_sp0250_latch;
};


/*----------- defined in audio/gottlieb.c -----------*/

#if USE_FAKE_VOTRAX
MACHINE_CONFIG_EXTERN( reactor_samples );
MACHINE_CONFIG_EXTERN( qbert_samples );
#endif
