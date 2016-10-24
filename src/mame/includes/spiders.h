// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Sigma Spiders hardware

***************************************************************************/
#include "sound/discrete.h"
#include "video/mc6845.h"

class spiders_state : public driver_device
{
public:
	spiders_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_audiocpu(*this, "audiocpu") { }

	required_shared_ptr<uint8_t> m_ram;
	required_device<discrete_device> m_discrete;
	uint8_t m_flipscreen;
	uint16_t m_gfx_rom_address;
	uint8_t m_gfx_rom_ctrl_mode;
	uint8_t m_gfx_rom_ctrl_latch;
	uint8_t m_gfx_rom_ctrl_data;

	void main_cpu_irq(int state);
	void flipscreen_w(int state);
	void display_enable_changed(int state);
	void gfx_rom_intf_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gfx_rom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	void update_pia_1(device_t &device);
	void ic60_74123_output_changed(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spiders_audio_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spiders_audio_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spiders_audio_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spiders_audio_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	MC6845_UPDATE_ROW(crtc_update_row);

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_audiocpu;
};

/*----------- defined in audio/spiders.c -----------*/
MACHINE_CONFIG_EXTERN( spiders_audio );
