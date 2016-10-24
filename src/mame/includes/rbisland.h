// license:BSD-3-Clause
// copyright-holders:Mike Coates
/*************************************************************************

    Rainbow Islands

*************************************************************************/

#include "video/pc080sn.h"
#include "video/pc090oj.h"

class rbisland_state : public driver_device
{
public:
	rbisland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_pc080sn(*this, "pc080sn"),
		m_pc090oj(*this, "pc090oj"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	optional_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	uint16_t      m_sprite_ctrl;
	uint16_t      m_sprites_flipscreen;

	/* misc */
	uint8_t       m_jumping_latch;

	/* c-chip */
	std::unique_ptr<uint8_t[]>    m_CRAM[8];
	int         m_extra_version;
	uint8_t       m_current_bank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<pc080sn_device> m_pc080sn;
	optional_device<pc090oj_device> m_pc090oj;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void jumping_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t jumping_latch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rbisland_cchip_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rbisland_cchip_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rbisland_cchip_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t rbisland_cchip_ctrl_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t rbisland_cchip_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void rbisland_spritectrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void jumping_spritectrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_jumping();
	void init_rbislande();
	void init_rbisland();
	virtual void machine_start() override;
	void video_start_jumping();
	uint32_t screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jumping(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cchip_timer(void *ptr, int32_t param);
	void request_round_data(  );
	void request_world_data(  );
	void request_goalin_data(  );
	void rbisland_cchip_init( int version );
};
