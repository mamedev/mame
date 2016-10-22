// license:BSD-3-Clause
// copyright-holders:Mark McDougall
#include "sound/msm5205.h"
#include "video/stfight_dev.h"
#include "video/airraid_dev.h"

class stfight_state : public driver_device
{
public:
	enum
	{
		TIMER_STFIGHT_INTERRUPT_1
	};

	stfight_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_msm(*this, "msm"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_stfight_video(*this, "stfight_vid"),
		m_airraid_video(*this, "airraid_vid")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_mcu;
	required_device<msm5205_device> m_msm;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	optional_device<stfight_video_device> m_stfight_video;
	optional_device<airraid_video_device> m_airraid_video;

	uint8_t *m_decrypt;
	uint8_t m_fm_data;
	uint8_t m_cpu_to_mcu_data;
	uint8_t m_cpu_to_mcu_empty;

	uint16_t m_adpcm_data_offs;
	uint8_t m_adpcm_nibble;
	uint8_t m_adpcm_reset;

	uint8_t m_coin_state;

	DECLARE_WRITE_LINE_MEMBER(stfight_adpcm_int);

	DECLARE_DRIVER_INIT(stfight);
	DECLARE_DRIVER_INIT(empcity);
	DECLARE_DRIVER_INIT(cshooter);

	DECLARE_WRITE8_MEMBER(stfight_io_w);
	DECLARE_READ8_MEMBER(stfight_coin_r);
	DECLARE_WRITE8_MEMBER(stfight_coin_w);
	DECLARE_WRITE8_MEMBER(stfight_fm_w);
	DECLARE_WRITE8_MEMBER(stfight_mcu_w);

	DECLARE_WRITE8_MEMBER(stfight_bank_w);


	DECLARE_READ8_MEMBER(stfight_fm_r);


	virtual void machine_start() override;
	virtual void machine_reset() override;

	INTERRUPT_GEN_MEMBER(stfight_vb_interrupt);

	/*
	    MCU specifics
	*/

	DECLARE_READ8_MEMBER(stfight_68705_port_a_r);
	DECLARE_READ8_MEMBER(stfight_68705_port_b_r);
	DECLARE_READ8_MEMBER(stfight_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(stfight_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(stfight_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(stfight_68705_port_c_w);

	DECLARE_WRITE8_MEMBER(stfight_68705_ddr_a_w);
	DECLARE_WRITE8_MEMBER(stfight_68705_ddr_b_w);
	DECLARE_WRITE8_MEMBER(stfight_68705_ddr_c_w);

	uint8_t m_portA_out, m_portA_in;
	uint8_t m_portB_out, m_portB_in;
	uint8_t m_portC_out, m_portC_in;
	uint8_t m_ddrA, m_ddrB, m_ddrC;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
