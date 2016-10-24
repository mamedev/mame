// license:LGPL-2.1+
// copyright-holders:Ville Linde, Angelo Salese, hap
/*************************************************************************

  Taito JC System

*************************************************************************/

#include "video/tc0780fpa.h"
#include "machine/taitoio.h"

class taitojc_state : public driver_device
{
public:
	taitojc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_dsp(*this,"dsp"),
		m_tc0640fio(*this, "tc0640fio"),
		m_gfx2(*this, "gfx2"),
		m_vram(*this, "vram"),
		m_objlist(*this, "objlist"),
		m_snd_shared_ram(*this, "snd_shared"),
		m_main_ram(*this, "main_ram"),
		m_dsp_shared_ram(*this, "dsp_shared"),
		m_palette_ram(*this, "palette_ram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_analog_ports(*this, "AN.%u", 0),
		m_tc0780fpa(*this, "tc0780fpa")
	{
		m_mcu_output = 0;
		m_speed_meter = 0;
		m_brake_meter = 0;
	}

	// device/memory pointers
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_dsp;
	required_device<tc0640fio_device> m_tc0640fio;
	required_memory_region m_gfx2;

	required_shared_ptr<uint32_t> m_vram;
	required_shared_ptr<uint32_t> m_objlist;
	required_shared_ptr<uint32_t> m_snd_shared_ram;
	required_shared_ptr<uint32_t> m_main_ram;
	required_shared_ptr<uint16_t> m_dsp_shared_ram;
	required_shared_ptr<uint32_t> m_palette_ram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_ioport_array<8> m_analog_ports;

	required_device<tc0780fpa_device> m_tc0780fpa;

	uint32_t m_dsp_rom_pos;

	int m_first_dsp_reset;
	int16_t m_viewport_data[3];
	int16_t m_projection_data[3];
	int16_t m_intersection_data[3];

	int m_gfx_index;

	std::unique_ptr<uint32_t[]> m_char_ram;
	std::unique_ptr<uint32_t[]> m_tile_ram;
	tilemap_t *m_tilemap;

	uint8_t m_mcu_comm_main;
	uint8_t m_mcu_comm_hc11;
	uint8_t m_mcu_data_main;
	uint8_t m_mcu_data_hc11;
	uint8_t m_mcu_output;

	uint8_t m_has_dsp_hack;

	int m_speed_meter;
	int m_brake_meter;

	uint8_t mcu_comm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_comm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t snd_share_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void snd_share_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t jc_pcbid_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t jc_lan_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void jc_lan_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jc_irq_unk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dendego_speedmeter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dendego_brakemeter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t hc11_comm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hc11_comm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hc11_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hc11_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hc11_output_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hc11_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hc11_analog_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	uint16_t dsp_shared_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_shared_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_to_main_7fe_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_to_main_7fe_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void main_to_dsp_7ff_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t dsp_rom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_rom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void dsp_math_viewport_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dsp_math_projection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_math_projection_y_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dsp_math_projection_x_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_math_intersection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_math_intersection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dsp_math_unk_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	uint16_t taitojc_dsp_idle_skip_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dendego2_dsp_idle_skip_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	uint32_t taitojc_palette_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void taitojc_palette_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t taitojc_tile_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t taitojc_char_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void taitojc_tile_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void taitojc_char_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void init_dendego2();
	void init_dangcurv();
	void init_taitojc();
	void taitojc_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update_taitojc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dendego(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void taitojc_vblank(device_t &device);
	void draw_object(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t w1, uint32_t w2, uint8_t bank_type);
	void draw_object_bank(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t bank_type, uint8_t pri);
};
