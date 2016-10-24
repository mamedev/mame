// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina
#include "audio/seibu.h"
#include "machine/seibucop/seibucop.h"
#include "video/seibu_crtc.h"

class raiden2_state : public driver_device
{
public:
	raiden2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		/*
		  back_data(*this, "back_data"),
		  fore_data(*this, "fore_data"),
		  mid_data(*this, "mid_data"),
		  text_data(*this, "text_data"),
		  */
			sprites(*this, "sprites") ,
			m_maincpu(*this, "maincpu"),
			m_seibu_sound(*this, "seibu_sound"),
			m_gfxdecode(*this, "gfxdecode"),
			m_palette(*this, "palette"),

			bg_bank(0),
			fg_bank(0),
			mid_bank(0),
			tx_bank(0),
			raiden2_tilemap_enable(0),
			prg_bank(0),
			cop_bank(0),

			sprite_prot_x(0),
			sprite_prot_y(0),
			dst1(0),
			cop_spr_maxx(0),
			cop_spr_off(0),

			tile_buffer(320, 256),
			sprite_buffer(320, 256),
			m_raiden2cop(*this, "raiden2cop")
	{
		memset(scrollvals, 0, sizeof(uint16_t)*6);
		memset(sprite_prot_src_addr, 0, sizeof(uint16_t)*2);

	}

	std::unique_ptr<uint16_t[]> back_data;
	std::unique_ptr<uint16_t[]> fore_data;
	std::unique_ptr<uint16_t[]> mid_data;
	std::unique_ptr<uint16_t[]> text_data; // private buffers, allocated in init
	required_shared_ptr<uint16_t> sprites;
	required_device<cpu_device> m_maincpu;
	optional_device<seibu_sound_device> m_seibu_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;




	void raiden2_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t cop_tile_bank_2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void cop_tile_bank_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void raidendx_cop_bank_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tilemap_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tile_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tile_bank_01_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void raiden2_background_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void raiden2_foreground_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void raiden2_midground_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void raiden2_text_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void m_videoram_private_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void sprcpt_val_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprcpt_val_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprcpt_data_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprcpt_data_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprcpt_data_3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprcpt_data_4_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprcpt_adr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprcpt_flags_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprcpt_flags_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t raiden2_sound_comms_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void raiden2_sound_comms_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void common_reset();

	static uint16_t const raiden_blended_colors[];
	static uint16_t const xsedae_blended_colors[];
	static uint16_t const zeroteam_blended_colors[];

	bool blend_active[0x800]; // cfg

	tilemap_t *background_layer,*midground_layer,*foreground_layer,*text_layer;


	int bg_bank, fg_bank, mid_bank, tx_bank;
	uint16_t raiden2_tilemap_enable;
	uint8_t prg_bank;
	uint16_t cop_bank;

	uint16_t scrollvals[6];




	void sprite_prot_x_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprite_prot_y_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprite_prot_src_seg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprite_prot_src_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sprite_prot_src_seg_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t sprite_prot_dst1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t sprite_prot_maxx_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t sprite_prot_off_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sprite_prot_dst1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprite_prot_maxx_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprite_prot_off_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t sprite_prot_x,sprite_prot_y,dst1,cop_spr_maxx,cop_spr_off;
	uint16_t sprite_prot_src_addr[2];



	void draw_sprites(const rectangle &cliprect);



	const int *cur_spri; // cfg

	void init_raidendx();
	void init_xsedae();
	void init_zeroteam();
	void init_raiden2();
	void get_back_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_mid_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fore_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_reset_raiden2();
	void video_start_raiden2();
	void machine_reset_zeroteam();
	void machine_reset_xsedae();
	void machine_reset_raidendx();
	uint32_t screen_update_raiden2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void raiden2_interrupt(device_t &device);
	void combine32(uint32_t *val, int offset, uint16_t data, uint16_t mem_mask);
	void sprcpt_init(void);

	void blend_layer(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind16 &source, int layer);
	void tilemap_draw_and_blend(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tilemap);

	void init_blending(const uint16_t *table);

	bitmap_ind16 tile_buffer, sprite_buffer;
	optional_device<raiden2cop_device> m_raiden2cop;

protected:
	virtual void machine_start() override;
};
