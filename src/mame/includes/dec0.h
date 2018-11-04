// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "machine/74157.h"
#include "machine/bankdev.h"
#include "machine/gen_latch.h"
#include "video/decbac06.h"
#include "video/decmxc06.h"
#include "sound/msm5205.h"

class dec0_state : public driver_device
{
public:
	enum class mcu_type {
		EMULATED,
		BADDUDES_SIM,
		BIRDTRY_SIM
	};

	dec0_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_mcu(*this, "mcu"),
		m_palette(*this, "palette"),
		m_tilegen1(*this, "tilegen1"),
		m_tilegen2(*this, "tilegen2"),
		m_tilegen3(*this, "tilegen3"),
		m_spritegen(*this, "spritegen"),
		m_pfprotect(*this, "pfprotect"),
		m_sndprotect(*this, "sndprotect"),
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
	required_device<palette_device> m_palette;
	optional_device<deco_bac06_device> m_tilegen1;
	optional_device<deco_bac06_device> m_tilegen2;
	optional_device<deco_bac06_device> m_tilegen3;
	optional_device<deco_mxc06_device> m_spritegen;
	optional_device<address_map_bank_device> m_pfprotect;
	optional_device<address_map_bank_device> m_sndprotect;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_ram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_paletteram;
	optional_shared_ptr<uint8_t> m_robocop_shared_ram;
	optional_shared_ptr<uint8_t> m_hippodrm_shared_ram;

	mcu_type m_game;
	uint16_t m_i8751_return;
	uint16_t m_i8751_command;
	int m_slyspy_state;
	int m_slyspy_sound_state;
	int m_hippodrm_msb;
	int m_hippodrm_lsb;
	uint8_t m_i8751_ports[4];
	uint16_t *m_buffered_spriteram;
	uint16_t m_pri;

	DECLARE_WRITE16_MEMBER(dec0_control_w);
	DECLARE_WRITE16_MEMBER(midres_sound_w);
	DECLARE_READ16_MEMBER(slyspy_controls_r);
	DECLARE_READ16_MEMBER(slyspy_protection_r);
	DECLARE_WRITE16_MEMBER(slyspy_state_w);
	DECLARE_READ16_MEMBER(slyspy_state_r);
	DECLARE_READ16_MEMBER(dec0_controls_r);
	DECLARE_READ16_MEMBER(midres_controls_r);
	DECLARE_READ8_MEMBER(hippodrm_prot_r);
	DECLARE_WRITE8_MEMBER(hippodrm_prot_w);
	DECLARE_READ8_MEMBER(dec0_mcu_port_r);
	DECLARE_WRITE8_MEMBER(dec0_mcu_port_w);
	DECLARE_READ16_MEMBER(hippodrm_68000_share_r);
	DECLARE_WRITE16_MEMBER(hippodrm_68000_share_w);
	DECLARE_WRITE16_MEMBER(sprite_mirror_w);
	DECLARE_READ16_MEMBER(robocop_68000_share_r);
	DECLARE_WRITE16_MEMBER(robocop_68000_share_w);
	DECLARE_WRITE16_MEMBER(dec0_update_sprites_w);
	DECLARE_WRITE16_MEMBER(dec0_priority_w);
	DECLARE_READ16_MEMBER(ffantasybl_242024_r);

	DECLARE_READ8_MEMBER(slyspy_sound_state_r);
	DECLARE_READ8_MEMBER(slyspy_sound_state_reset_r);

	DECLARE_DRIVER_INIT(robocop);
	DECLARE_DRIVER_INIT(hippodrm);
	DECLARE_DRIVER_INIT(hbarrel);
	DECLARE_DRIVER_INIT(slyspy);
	DECLARE_DRIVER_INIT(birdtry);
	DECLARE_DRIVER_INIT(drgninja);
	DECLARE_DRIVER_INIT(midresb);
	DECLARE_DRIVER_INIT(ffantasybl);

	virtual void machine_start() override;
	DECLARE_MACHINE_RESET(slyspy);
	DECLARE_VIDEO_START(dec0);
	DECLARE_VIDEO_START(dec0_nodma);

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
	void dec0_base(machine_config &config);
	void dec0(machine_config &config);
	void dec1(machine_config &config);
	void midres(machine_config &config);
	void birdtry(machine_config &config);
	void baddudes(machine_config &config);
	void midresbj(machine_config &config);
	void slyspy(machine_config &config);
	void hbarrel(machine_config &config);
	void midresb(machine_config &config);
	void ffantasybl(machine_config &config);
	void drgninjab(machine_config &config);
	void robocop(machine_config &config);
	void robocopb(machine_config &config);
	void hippodrm(machine_config &config);
	void dec0_map(address_map &map);
	void dec0_s_map(address_map &map);
	void hippodrm_sub_map(address_map &map);
	void mcu_io_map(address_map &map);
	void midres_map(address_map &map);
	void midres_s_map(address_map &map);
	void midresb_map(address_map &map);
	void robocop_sub_map(address_map &map);
	void slyspy_map(address_map &map);
	void slyspy_protection_map(address_map &map);
	void slyspy_s_map(address_map &map);
	void slyspy_sound_protection_map(address_map &map);
};


class dec0_automat_state : public dec0_state
{
public:
	dec0_automat_state(const machine_config &mconfig, device_type type, const char *tag)
		: dec0_state(mconfig, type, tag),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_adpcm_select1(*this, "adpcm_select1"),
		m_adpcm_select2(*this, "adpcm_select2"),
		m_soundbank(*this, "soundbank")
	{
	}

	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
	required_device<ls157_device> m_adpcm_select1;
	required_device<ls157_device> m_adpcm_select2;
	required_memory_bank m_soundbank;

	bool m_adpcm_toggle1;
	bool m_adpcm_toggle2;
	uint16_t m_automat_scroll_regs[4];

	DECLARE_WRITE16_MEMBER(automat_control_w);
	DECLARE_READ16_MEMBER( automat_palette_r );
	DECLARE_WRITE16_MEMBER( automat_palette_w );
	DECLARE_WRITE16_MEMBER( automat_scroll_w )
	{
		COMBINE_DATA(&m_automat_scroll_regs[offset]);
	}
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE_LINE_MEMBER(msm1_vclk_cb);
	DECLARE_WRITE_LINE_MEMBER(msm2_vclk_cb);

	virtual void machine_start() override;

	uint32_t screen_update_automat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_secretab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void secretab(machine_config &config);
	void automat(machine_config &config);
	void automat_map(address_map &map);
	void automat_s_map(address_map &map);
	void secretab_map(address_map &map);
	void secretab_s_map(address_map &map);
};
