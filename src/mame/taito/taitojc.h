// license:LGPL-2.1+
// copyright-holders:Ville Linde, Angelo Salese, hap
/*************************************************************************

  Taito JC System

*************************************************************************/

#include "tc0780fpa.h"
#include "taitoio.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class taitojc_state : public driver_device
{
public:
	taitojc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_dsp(*this,"dsp"),
		m_tc0640fio(*this, "tc0640fio"),
		m_dspgfx(*this, "dspgfx"),
		m_vram(*this, "vram"),
		m_objlist(*this, "objlist"),
		m_main_ram(*this, "main_ram"),
		m_dsp_shared_ram(*this, "dsp_shared"),
		m_palette_ram(*this, "palette_ram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_analog_ports(*this, "AN.%u", 0),
		m_tc0780fpa(*this, "tc0780fpa"),
		m_lamps(*this, "lamp%u", 0U),
		m_counters(*this, "counter%u", 0U)
	{
		m_speed_meter = 0;
		m_brake_meter = 0;
	}

	void taitojc(machine_config &config);
	void dendego(machine_config &config);

	void init_dendego2();
	void init_dangcurv();
	void init_taitojc();

private:
	// device/memory pointers
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_dsp;
	required_device<tc0640fio_device> m_tc0640fio;
	required_region_ptr<uint16_t> m_dspgfx;

	required_shared_ptr<uint32_t> m_vram;
	required_shared_ptr<uint32_t> m_objlist;
	required_shared_ptr<uint32_t> m_main_ram;
	required_shared_ptr<uint16_t> m_dsp_shared_ram;
	required_shared_ptr<uint32_t> m_palette_ram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_ioport_array<8> m_analog_ports;

	required_device<tc0780fpa_device> m_tc0780fpa;

	output_finder<8> m_lamps;
	output_finder<5> m_counters;

	uint32_t m_dsp_rom_pos = 0;

	int m_first_dsp_reset = 0;
	int16_t m_viewport_data[3];
	int16_t m_projection_data[3];
	int16_t m_intersection_data[3];

	int m_gfx_index = 0;

	std::unique_ptr<uint32_t[]> m_char_ram;
	std::unique_ptr<uint32_t[]> m_tile_ram;
	tilemap_t *m_tilemap = nullptr;

	uint8_t m_mcu_comm_main = 0;
	uint8_t m_mcu_comm_hc11 = 0;
	uint8_t m_mcu_data_main = 0;
	uint8_t m_mcu_data_hc11 = 0;

	uint8_t m_has_dsp_hack = 0;

	int m_speed_meter = 0;
	int m_brake_meter = 0;

	void coin_control_w(uint8_t data);

	uint8_t mcu_comm_r(offs_t offset);
	void mcu_comm_w(offs_t offset, uint8_t data);
	uint8_t jc_pcbid_r(offs_t offset);
	uint8_t jc_lan_r();
	void jc_lan_w(uint8_t data);
	void jc_irq_unk_w(uint8_t data);
	void dendego_speedmeter_w(uint8_t data);
	void dendego_brakemeter_w(uint8_t data);

	uint8_t hc11_comm_r();
	void hc11_comm_w(uint8_t data);
	void hc11_output_w(uint8_t data);
	uint8_t hc11_data_r();
	void hc11_data_w(uint8_t data);
	template <int Ch> uint8_t hc11_analog_r();

	uint16_t dsp_shared_r(offs_t offset);
	void dsp_shared_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_to_main_7fe_r(offs_t offset, uint16_t mem_mask = ~0);
	void dsp_to_main_7fe_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void main_to_dsp_7ff_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t dsp_rom_r();
	void dsp_rom_w(offs_t offset, uint16_t data);

	void dsp_math_viewport_w(offs_t offset, uint16_t data);
	void dsp_math_projection_w(offs_t offset, uint16_t data);
	uint16_t dsp_math_projection_y_r();
	uint16_t dsp_math_projection_x_r();
	void dsp_math_intersection_w(offs_t offset, uint16_t data);
	uint16_t dsp_math_intersection_r();
	uint16_t dsp_math_unk_r();

	uint16_t taitojc_dsp_idle_skip_r();
	uint16_t dendego2_dsp_idle_skip_r();

	uint32_t taitojc_palette_r(offs_t offset);
	void taitojc_palette_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t taitojc_tile_r(offs_t offset);
	uint32_t taitojc_char_r(offs_t offset);
	void taitojc_tile_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void taitojc_char_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(taitojc_tile_info);
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_taitojc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dendego(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(taitojc_vblank);
	void draw_object(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t w1, uint32_t w2, uint8_t bank_type);
	void draw_object_bank(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t bank_type, uint8_t pri);

	void dendego_map(address_map &map) ATTR_COLD;
	void hc11_pgm_map(address_map &map) ATTR_COLD;
	void taitojc_map(address_map &map) ATTR_COLD;
	void tms_data_map(address_map &map) ATTR_COLD;
	void tms_program_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;
};
