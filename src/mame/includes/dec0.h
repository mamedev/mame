// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "machine/bankdev.h"
#include "machine/gen_latch.h"
#include "video/decbac06.h"
#include "video/decmxc06.h"
#include "sound/msm5205.h"

class dec0_state : public driver_device
{
public:
	dec0_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_mcu(*this, "mcu"),
		m_msm(*this, "msm"),
		m_palette(*this, "palette"),
		m_tilegen1(*this, "tilegen1"),
		m_tilegen2(*this, "tilegen2"),
		m_tilegen3(*this, "tilegen3"),
		m_spritegen(*this, "spritegen"),
		m_pfprotect(*this, "pfprotect"),
		m_soundlatch(*this, "soundlatch"),
		m_ram(*this, "ram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "palette"),
		m_robocop_shared_ram(*this, "robocop_shared"),
		m_hippodrm_shared_ram(*this, "hippodrm_shared") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_mcu;
	optional_device<msm5205_device> m_msm;
	required_device<palette_device> m_palette;
	optional_device<deco_bac06_device> m_tilegen1;
	optional_device<deco_bac06_device> m_tilegen2;
	optional_device<deco_bac06_device> m_tilegen3;
	optional_device<deco_mxc06_device> m_spritegen;
	optional_device<address_map_bank_device> m_pfprotect;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_ram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_paletteram;
	optional_shared_ptr<uint8_t> m_robocop_shared_ram;
	optional_shared_ptr<uint8_t> m_hippodrm_shared_ram;

	int m_game;
	int m_i8751_return;
	int m_i8751_command;
	int m_slyspy_state;
	int m_hippodrm_msb;
	int m_hippodrm_lsb;
	uint8_t m_i8751_ports[4];
	uint16_t *m_buffered_spriteram;
	uint16_t m_pri;

	void dec0_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void slyspy_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void midres_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t slyspy_controls_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t slyspy_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void slyspy_state_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t slyspy_state_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dec0_controls_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dec0_rotary_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t midres_controls_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t hippodrm_prot_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hippodrm_prot_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dec0_mcu_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dec0_mcu_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t hippodrm_68000_share_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void hippodrm_68000_share_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sprite_mirror_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t robocop_68000_share_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void robocop_68000_share_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dec0_update_sprites_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dec0_priority_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ffantasybl_242024_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void init_robocop();
	void init_hippodrm();
	void init_hbarrel();
	void init_slyspy();
	void init_birdtry();
	void init_baddudes();
	void init_midresb();
	void init_ffantasybl();

	virtual void machine_start() override;
	void machine_reset_slyspy();
	void video_start_dec0();
	void video_start_dec0_nodma();

	uint32_t screen_update_hbarrel(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_baddudes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_birdtry(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_robocop(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hippodrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_slyspy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_midres(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void baddudes_i8751_write(int data);
	void birdtry_i8751_write(int data);
	void dec0_i8751_write(int data);
	void dec0_i8751_reset();
	void h6280_decrypt(const char *cputag);
};


class dec0_automat_state : public dec0_state
{
public:
	dec0_automat_state(const machine_config &mconfig, device_type type, const char *tag)
		: dec0_state(mconfig, type, tag) {
	}

	uint8_t m_automat_adpcm_byte;
	int m_automat_msm5205_vclk_toggle;
	uint16_t m_automat_scroll_regs[4];

	void automat_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void automat_adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t automat_palette_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void automat_palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void automat_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff)
	{
		COMBINE_DATA(&m_automat_scroll_regs[offset]);
	}
	void automat_vclk_cb(int state);

	virtual void machine_start() override;

	uint32_t screen_update_automat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_secretab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
