// license:BSD-3-Clause
// copyright-holders:Luca Elia
#include "cpu/upd7725/upd7725.h"
#include "video/st0020.h"
#include "machine/eepromser.h"
#include "sound/es5506.h"

class ssv_state : public driver_device
{
public:
	ssv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ensoniq(*this, "ensoniq"),
		m_eeprom(*this, "eeprom"),
		m_dsp(*this, "dsp"),
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_irq_vectors(*this, "irq_vectors"),
		m_gdfs_tmapram(*this, "gdfs_tmapram"),
		m_gdfs_tmapscroll(*this, "gdfs_tmapscroll"),
		m_gdfs_st0020(*this, "st0020_spr"),
		m_input_sel(*this, "input_sel"),
		m_io_gun(*this, {"GUNX1", "GUNY1", "GUNX2", "GUNY2"}),
		m_io_key0(*this, "KEY0"),
		m_io_key1(*this, "KEY1"),
		m_io_key2(*this, "KEY2"),
		m_io_key3(*this, "KEY3"),
		m_io_service(*this, "SERVICE"),
		m_io_paddle(*this, "PADDLE"),
		m_io_trackx(*this, "TRACKX"),
		m_io_tracky(*this, "TRACKY"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<es5506_device> m_ensoniq;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<upd96050_device> m_dsp;

	required_shared_ptr<uint16_t> m_mainram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_scroll;
	required_shared_ptr<uint16_t> m_irq_vectors;
	optional_shared_ptr<uint16_t> m_gdfs_tmapram;
	optional_shared_ptr<uint16_t> m_gdfs_tmapscroll;
	optional_device<st0020_device> m_gdfs_st0020;
	optional_shared_ptr<uint16_t> m_input_sel;

	int m_tile_code[16];
	int m_enable_video;
	int m_shadow_pen_mask;
	int m_shadow_pen_shift;
	uint8_t m_requested_int;
	uint16_t m_irq_enable;
	std::unique_ptr<uint16_t[]> m_eaglshot_gfxram;
	tilemap_t *m_gdfs_tmap;
	int m_interrupt_ultrax;
	int m_gdfs_lightgun_select;
	uint16_t m_sxyreact_serial;
	int m_sxyreact_dial;
	uint16_t m_gdfs_eeprom_old;
	uint32_t m_latches[8];
	uint8_t m_trackball_select;

	void irq_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void irq_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void lockout_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void lockout_inv_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_dr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_dr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t drifto94_unknown_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t hypreact_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t mainram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mainram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t srmp4_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t srmp7_irqv_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void srmp7_sound_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t srmp7_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t sxyreact_ballswitch_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t sxyreact_dial_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sxyreact_dial_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sxyreact_motor_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t latch32_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void latch32_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t latch16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void latch16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void eaglshot_gfxrom_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t eaglshot_trackball_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void eaglshot_trackball_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t eaglshot_gfxram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void eaglshot_gfxram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gdfs_tmapram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t vblank_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t gdfs_eeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void gdfs_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void get_tile_info_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void init_gdfs();
	void init_sxyreac2();
	void init_hypreac2();
	void init_hypreact();
	void init_dynagear();
	void init_eaglshot();
	void init_srmp4();
	void init_srmp7();
	void init_keithlcy();
	void init_meosism();
	void init_vasara();
	void init_cairblad();
	void init_sxyreact();
	void init_janjans1();
	void init_ryorioh();
	void init_drifto94();
	void init_survarts();
	void init_ultrax();
	void init_stmblade();
	void init_jsk();
	void init_twineag2();
	void init_mslider();
	virtual void machine_reset() override;
	virtual void video_start() override;
	void video_start_gdfs();
	void video_start_eaglshot();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gdfs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_eaglshot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void interrupt(timer_device &timer, void *ptr, int32_t param);
	void gdfs_interrupt(timer_device &timer, void *ptr, int32_t param);
	void update_irq_state();
	int irq_callback(device_t &device, int irqline);

	void drawgfx(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx,uint32_t code,uint32_t color,int flipx,int flipy,int x0,int y0,int shadow);
	void draw_row(bitmap_ind16 &bitmap, const rectangle &cliprect, int sx, int sy, int scroll);
	void draw_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, int  nr);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void enable_video(int enable);
	void init(int interrupt_ultrax);
	void init_hypreac2_common();
	void init_eaglshot_banking();
	void init_st010();

protected:
	optional_ioport_array<4> m_io_gun;
	optional_ioport m_io_key0;
	optional_ioport m_io_key1;
	optional_ioport m_io_key2;
	optional_ioport m_io_key3;
	optional_ioport m_io_service;
	optional_ioport m_io_paddle;
	optional_ioport m_io_trackx;
	optional_ioport m_io_tracky;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
