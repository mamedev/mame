// license:BSD-3-Clause
// copyright-holders:Philip Bennett,Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino, Ernesto Corvi, David Haywood, R. Belmont
/*************************************************************************

    Double Dragon & Double Dragon II (but also China Gate)

*************************************************************************/

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class ddragon_state : public driver_device
{
public:
	ddragon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_rambase(*this, "rambase"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_comram(*this, "comram"),
		m_spriteram(*this, "spriteram"),
		m_scrollx_lo(*this, "scrollx_lo"),
		m_scrolly_lo(*this, "scrolly_lo"),
		m_darktowr_mcu_ports(*this, "darktowr_mcu"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_subcpu(*this, "sub"),
		m_adpcm1(*this, "adpcm1"),
		m_adpcm2(*this, "adpcm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	optional_shared_ptr<uint8_t> m_rambase;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	optional_shared_ptr<uint8_t> m_comram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scrollx_lo;
	required_shared_ptr<uint8_t> m_scrolly_lo;
	optional_shared_ptr<uint8_t> m_darktowr_mcu_ports;

	/* video-related */
	tilemap_t      *m_fg_tilemap;
	tilemap_t      *m_bg_tilemap;
	uint8_t          m_technos_video_hw;
	uint8_t          m_scrollx_hi;
	uint8_t          m_scrolly_hi;

	/* misc */
	uint8_t          m_ddragon_sub_port;
	uint8_t          m_sprite_irq;
	uint8_t          m_sound_irq;
	uint8_t          m_ym_irq;
	uint8_t          m_adpcm_sound_irq;
	uint32_t         m_adpcm_pos[2];
	uint32_t         m_adpcm_end[2];
	uint8_t          m_adpcm_idle[2];
	int            m_adpcm_data[2];

	/* for Sai Yu Gou Ma Roku */
	int            m_adpcm_addr;
	int            m_i8748_P1;
	int            m_i8748_P2;
	int            m_pcm_shift;
	int            m_pcm_nibble;
	int            m_mcu_command;
#if 0
	int            m_m5205_clk;
#endif

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<cpu_device> m_subcpu;
	optional_device<msm5205_device> m_adpcm1;
	optional_device<msm5205_device> m_adpcm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;


	int scanline_to_vcount(int scanline);
	void ddragon_interrupt_ack(address_space &space, offs_t offset, uint8_t data);
	void dd_adpcm_int(msm5205_device *device, int chip);

	/* video/ddragon.c */
	tilemap_memory_index background_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_16color_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	uint32_t screen_update_ddragon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void machine_start_ddragon();
	void machine_reset_ddragon();
	void video_start_ddragon();

	void ddragon_scanline(timer_device &timer, void *ptr, int32_t param);

	void irq_handler(int state);
	void ddragon_bgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ddragon_fgvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value subcpu_bus_free(ioport_field &field, void *param);
	void ddragon_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void toffy_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t darktowr_mcu_bank_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t soundlatch_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void darktowr_mcu_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void darktowr_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ddragon_interrupt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ddragon_interrupt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ddragon2_sub_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ddragon2_sub_irq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void darktowr_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ddragon_hd63701_internal_registers_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ddragon_hd63701_internal_registers_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ddragon_comram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ddragon_comram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dd_adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dd_adpcm_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ddragonba_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dd_adpcm_int_1(int state);
	void dd_adpcm_int_2(int state);

	void init_toffy();
	void init_darktowr();
	void init_ddragon2();
	void init_ddragon();
	void init_ddragon6809();
};
