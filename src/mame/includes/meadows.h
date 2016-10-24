// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/*************************************************************************

    Meadows S2650 hardware

*************************************************************************/
#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "sound/dac.h"
#include "sound/samples.h"

class meadows_state : public driver_device
{
public:
	meadows_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dac(*this, "dac"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram")
	{
	}

	required_device<s2650_device> m_maincpu;
	optional_device<s2650_device> m_audiocpu;
	optional_device<dac_8bit_r2r_device> m_dac;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	int m_channel;
	int m_freq1;
	int m_freq2;
	uint8_t m_latched_0c01;
	uint8_t m_latched_0c02;
	uint8_t m_latched_0c03;
	uint8_t m_main_sense_state;
	uint8_t m_audio_sense_state;
	uint8_t m_0c00;
	uint8_t m_0c01;
	uint8_t m_0c02;
	uint8_t m_0c03;
	tilemap_t *m_bg_tilemap;
	uint8_t hsync_chain_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t vsync_chain_hi_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t vsync_chain_lo_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void meadows_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void audio_hardware_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t audio_hardware_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void meadows_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void meadows_spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void init_minferno();
	void init_gypsyjug();
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	uint32_t screen_update_meadows(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void meadows_interrupt(device_t &device);
	void minferno_interrupt(device_t &device);
	void audio_interrupt(device_t &device);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &clip);
	void meadows_sh_update();
	SAMPLES_START_CB_MEMBER(meadows_sh_start);
};
