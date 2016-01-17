// license:BSD-3-Clause
// copyright-holders:Couriersud
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "machine/netlist.h"

class irem_audio_device : public device_t
{
public:
	irem_audio_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	~irem_audio_device() {}

	DECLARE_WRITE8_MEMBER( cmd_w );
	DECLARE_WRITE8_MEMBER( m6803_port1_w );
	DECLARE_WRITE8_MEMBER( m6803_port2_w );
	DECLARE_READ8_MEMBER( m6803_port1_r );
	DECLARE_READ8_MEMBER( m6803_port2_r );
	DECLARE_WRITE8_MEMBER( ay8910_45M_portb_w );
	DECLARE_WRITE8_MEMBER( ay8910_45L_porta_w );
	DECLARE_WRITE8_MEMBER( sound_irq_ack_w );
	DECLARE_WRITE8_MEMBER( m52_adpcm_w );
	DECLARE_WRITE8_MEMBER( m62_adpcm_w );

	void adpcm_int(int st);

	netlist_mame_logic_input_t * m_audio_SINH;

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;

private:
	// internal state
	UINT8           m_port1;
	UINT8           m_port2;

	/* FIXME: using required / optional device would be better */
	// required_device<ay8910_device> m_ay_45L;
	// required_device<ay8910_device> m_ay_45M;
	ay8910_device *m_ay_45L;
	ay8910_device *m_ay_45M;
	msm5205_device *m_adpcm1;
	msm5205_device *m_adpcm2;

	netlist_mame_logic_input_t * m_audio_BD;
	netlist_mame_logic_input_t * m_audio_SD;
	netlist_mame_logic_input_t * m_audio_OH;
	netlist_mame_logic_input_t * m_audio_CH;
};

class m62_audio_device : public irem_audio_device
{
public:
	m62_audio_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const override;
};

class m52_soundc_audio_device : public irem_audio_device
{
public:
	m52_soundc_audio_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const override;
};

class m52_large_audio_device : public irem_audio_device
{
public:
	m52_large_audio_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const override;
};

//MACHINE_CONFIG_EXTERN( m52_sound_c_audio );
//MACHINE_CONFIG_EXTERN( m52_large_audio );
//MACHINE_CONFIG_EXTERN( m62_audio );

extern const device_type IREM_M62_AUDIO;
extern const device_type IREM_M52_SOUNDC_AUDIO;
extern const device_type IREM_M52_LARGE_AUDIO;
