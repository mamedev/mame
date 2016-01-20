// license:BSD-3-Clause
// copyright-holders:Luca Elia
/* TODO: some variables are per-game specifics */
#include "sound/okim6295.h"
#include "machine/ticket.h"

class cischeat_state : public driver_device
{
public:
	cischeat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vregs(*this, "vregs"),
		m_scrollram(*this, "scrollram"),
		m_ram(*this, "ram"),
		m_roadram(*this, "roadram"),
		m_f1gpstr2_ioready(*this, "ioready"),
		m_maincpu(*this, "maincpu"),
		m_cpu1(*this, "cpu1"),
		m_cpu2(*this, "cpu2"),
		m_cpu3(*this, "cpu3"),
		m_cpu5(*this, "cpu5"),
		m_soundcpu(*this, "soundcpu"),
		m_oki1(*this, "oki1"),
		m_oki2(*this, "oki2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_captflag_hopper(*this, "hopper"),
		m_captflag_motor_left(*this, "motor_left"),
		m_captflag_motor_right(*this, "motor_right"),
		m_oki1_bank(*this, "oki1_bank"),
		m_oki2_bank(*this, "oki2_bank")
		{
			for (int side = 0; side < 2; ++side)
				m_captflag_motor_command[side] = m_captflag_motor_pos[side] = 0;
			m_captflag_leds = 0;
		}

	required_shared_ptr<UINT16> m_vregs;
	optional_shared_ptr_array<UINT16,3> m_scrollram;
	required_shared_ptr<UINT16> m_ram;
	optional_shared_ptr_array<UINT16,2> m_roadram;
	optional_shared_ptr<UINT16> m_f1gpstr2_ioready;

	UINT16 *m_objectram;
	tilemap_t *m_tmap[3];
	tilemap_t *m_tilemap[3][2][4];
	int m_scrollx[3];
	int m_scrolly[3];
	int m_active_layers;
	int m_bits_per_color_code;
	int m_scroll_flag[3];

	int m_prev;
	int m_armold;
	UINT16 m_scudhamm_motor_command;
	int m_ip_select;
	UINT8 m_drawmode_table[16];
	int m_debugsprites;
	int m_show_unknown;
	UINT16 *m_spriteram;
	DECLARE_WRITE16_MEMBER(scudhamm_motor_command_w);
	DECLARE_WRITE16_MEMBER(scudhamm_leds_w);
	DECLARE_WRITE16_MEMBER(scudhamm_enable_w);
	DECLARE_WRITE16_MEMBER(scudhamm_oki_bank_w);
	DECLARE_READ16_MEMBER(armchmp2_motor_status_r);
	DECLARE_WRITE16_MEMBER(armchmp2_motor_command_w);
	DECLARE_READ16_MEMBER(armchmp2_analog_r);
	DECLARE_READ16_MEMBER(armchmp2_buttons_r);
	DECLARE_WRITE16_MEMBER(armchmp2_leds_w);
	DECLARE_WRITE16_MEMBER(bigrun_soundbank_w);
	DECLARE_READ16_MEMBER(f1gpstr2_io_r);
	DECLARE_WRITE16_MEMBER(f1gpstr2_io_w);
	DECLARE_READ16_MEMBER(scudhamm_motor_status_r);
	DECLARE_READ16_MEMBER(scudhamm_motor_pos_r);
	DECLARE_READ16_MEMBER(scudhamm_analog_r);
	DECLARE_WRITE16_MEMBER(cischeat_scrollram_0_w);
	DECLARE_WRITE16_MEMBER(cischeat_scrollram_1_w);
	DECLARE_WRITE16_MEMBER(cischeat_scrollram_2_w);
	DECLARE_READ16_MEMBER(bigrun_vregs_r);
	DECLARE_WRITE16_MEMBER(bigrun_vregs_w);
	DECLARE_READ16_MEMBER(cischeat_vregs_r);
	DECLARE_WRITE16_MEMBER(cischeat_vregs_w);
	DECLARE_READ16_MEMBER(f1gpstar_vregs_r);
	DECLARE_READ16_MEMBER(f1gpstr2_vregs_r);
	DECLARE_READ16_MEMBER(wildplt_vregs_r);
	DECLARE_WRITE16_MEMBER(f1gpstar_vregs_w);
	DECLARE_WRITE16_MEMBER(f1gpstr2_vregs_w);
	DECLARE_WRITE16_MEMBER(scudhamm_vregs_w);
	void cischeat_set_vreg_flag(int which, int data);
	DECLARE_WRITE16_MEMBER(cischeat_soundbank_1_w);
	DECLARE_WRITE16_MEMBER(cischeat_soundbank_2_w);
	DECLARE_DRIVER_INIT(wildplt);
	DECLARE_DRIVER_INIT(cischeat);
	DECLARE_DRIVER_INIT(bigrun);
	DECLARE_DRIVER_INIT(f1gpstar);
	TILEMAP_MAPPER_MEMBER(cischeat_scan_8x8);
	TILEMAP_MAPPER_MEMBER(cischeat_scan_16x16);
	TILE_GET_INFO_MEMBER(cischeat_get_scroll_tile_info_8x8);
	TILE_GET_INFO_MEMBER(cischeat_get_scroll_tile_info_16x16);
	DECLARE_VIDEO_START(bigrun);
	DECLARE_VIDEO_START(f1gpstar);
	DECLARE_VIDEO_START(cischeat);
	UINT32 screen_update_bigrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_scudhamm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cischeat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_f1gpstar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(bigrun_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(scudhamm_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(armchamp2_scanline);
	void prepare_shadows();
	inline void scrollram_w(address_space &space, offs_t offset, UINT16 data, UINT16 mem_mask, int which);
	void create_tilemaps();
	void cischeat_draw_road(bitmap_ind16 &bitmap, const rectangle &cliprect, int road_num, int priority1, int priority2, int transparency);
	void f1gpstar_draw_road(bitmap_ind16 &bitmap, const rectangle &cliprect, int road_num, int priority1, int priority2, int transparency);
	void cischeat_draw_sprites(bitmap_ind16 &bitmap , const rectangle &cliprect, int priority1, int priority2);
	void bigrun_draw_sprites(bitmap_ind16 &bitmap , const rectangle &cliprect, int priority1, int priority2);
	void cischeat_untangle_sprites(const char *region);
	optional_device<cpu_device> m_maincpu; // some are called cpu1
	optional_device<cpu_device> m_cpu1;
	optional_device<cpu_device> m_cpu2;
	optional_device<cpu_device> m_cpu3;
	optional_device<cpu_device> m_cpu5;
	optional_device<cpu_device> m_soundcpu;
	required_device<okim6295_device> m_oki1;
	required_device<okim6295_device> m_oki2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// captflag
	optional_device<ticket_dispenser_device> m_captflag_hopper;

	optional_device<timer_device> m_captflag_motor_left;
	optional_device<timer_device> m_captflag_motor_right;
	UINT16 m_captflag_motor_command[2];
	UINT16 m_captflag_motor_pos[2];

	DECLARE_WRITE16_MEMBER(captflag_motor_command_right_w);
	DECLARE_WRITE16_MEMBER(captflag_motor_command_left_w);
	void captflag_motor_move(int side, UINT16 data);
	DECLARE_CUSTOM_INPUT_MEMBER(captflag_motor_busy_r);
	DECLARE_CUSTOM_INPUT_MEMBER(captflag_motor_pos_r);

	optional_memory_bank m_oki1_bank;
	optional_memory_bank m_oki2_bank;
	DECLARE_WRITE16_MEMBER(captflag_oki_bank_w);

	UINT16 m_captflag_leds;
	DECLARE_WRITE16_MEMBER(captflag_leds_w);

	DECLARE_DRIVER_INIT(captflag);
	TIMER_DEVICE_CALLBACK_MEMBER(captflag_scanline);
};
