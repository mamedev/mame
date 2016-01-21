// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "machine/bankdev.h"
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

	required_shared_ptr<UINT16> m_ram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_paletteram;
	optional_shared_ptr<UINT8> m_robocop_shared_ram;
	optional_shared_ptr<UINT8> m_hippodrm_shared_ram;

	int m_game;
	int m_i8751_return;
	int m_i8751_command;
	int m_slyspy_state;
	int m_hippodrm_msb;
	int m_hippodrm_lsb;
	UINT8 m_i8751_ports[4];
	UINT16 *m_buffered_spriteram;
	UINT16 m_pri;

	DECLARE_WRITE16_MEMBER(dec0_control_w);
	DECLARE_WRITE16_MEMBER(slyspy_control_w);
	DECLARE_WRITE16_MEMBER(midres_sound_w);
	DECLARE_READ16_MEMBER(slyspy_controls_r);
	DECLARE_READ16_MEMBER(slyspy_protection_r);
	DECLARE_WRITE16_MEMBER(slyspy_state_w);
	DECLARE_READ16_MEMBER(slyspy_state_r);
	DECLARE_READ16_MEMBER(dec0_controls_r);
	DECLARE_READ16_MEMBER(dec0_rotary_r);
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
	DECLARE_WRITE_LINE_MEMBER(sound_irq);
	DECLARE_WRITE_LINE_MEMBER(sound_irq2);

	DECLARE_DRIVER_INIT(robocop);
	DECLARE_DRIVER_INIT(hippodrm);
	DECLARE_DRIVER_INIT(hbarrel);
	DECLARE_DRIVER_INIT(slyspy);
	DECLARE_DRIVER_INIT(birdtry);
	DECLARE_DRIVER_INIT(baddudes);
	DECLARE_DRIVER_INIT(midresb);
	DECLARE_DRIVER_INIT(ffantasybl);

	virtual void machine_start() override;
	DECLARE_MACHINE_RESET(slyspy);
	DECLARE_VIDEO_START(dec0);
	DECLARE_VIDEO_START(dec0_nodma);

	UINT32 screen_update_hbarrel(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_baddudes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_birdtry(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_robocop(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_hippodrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_slyspy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_midres(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

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

	UINT8 m_automat_adpcm_byte;
	int m_automat_msm5205_vclk_toggle;
	UINT16 m_automat_scroll_regs[4];

	DECLARE_WRITE16_MEMBER(automat_control_w);
	DECLARE_WRITE8_MEMBER(automat_adpcm_w);
	DECLARE_READ16_MEMBER( automat_palette_r );
	DECLARE_WRITE16_MEMBER( automat_palette_w );
	DECLARE_WRITE16_MEMBER( automat_scroll_w )
	{
		COMBINE_DATA(&m_automat_scroll_regs[offset]);
	}
	DECLARE_WRITE_LINE_MEMBER(automat_vclk_cb);

	virtual void machine_start() override;

	UINT32 screen_update_automat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_secretab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
