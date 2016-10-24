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

	void stfight_adpcm_int(int state);

	void init_stfight();
	void init_empcity();
	void init_cshooter();

	void stfight_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t stfight_coin_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void stfight_coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void stfight_fm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void stfight_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void stfight_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);


	uint8_t stfight_fm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);


	virtual void machine_start() override;
	virtual void machine_reset() override;

	void stfight_vb_interrupt(device_t &device);

	/*
	    MCU specifics
	*/

	uint8_t stfight_68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t stfight_68705_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t stfight_68705_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void stfight_68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void stfight_68705_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void stfight_68705_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void stfight_68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void stfight_68705_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void stfight_68705_ddr_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t m_portA_out, m_portA_in;
	uint8_t m_portB_out, m_portB_in;
	uint8_t m_portC_out, m_portC_in;
	uint8_t m_ddrA, m_ddrB, m_ddrC;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
