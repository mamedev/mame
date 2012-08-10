#include "video/decbac06.h"
#include "video/decmxc06.h"

class dec0_state : public driver_device
{
public:
	dec0_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_ram(*this, "ram"),
		m_spriteram(*this, "spriteram"),
		m_robocop_shared_ram(*this, "robocop_shared"),
		m_tilegen1(*this, "tilegen1"),
		m_tilegen2(*this, "tilegen2"),
		m_tilegen3(*this, "tilegen3"),
		m_spritegen(*this, "spritegen")
	{ }

	required_shared_ptr<UINT16> m_ram;
	required_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT8> m_robocop_shared_ram;

	optional_device<deco_bac06_device> m_tilegen1;
	optional_device<deco_bac06_device> m_tilegen2;
	optional_device<deco_bac06_device> m_tilegen3;
	optional_device<deco_mxc06_device> m_spritegen;


	int m_GAME;
	int m_i8751_return;
	int m_i8751_command;
	int m_slyspy_state;
	int m_share[0xff];
	int m_hippodrm_msb;
	int m_hippodrm_lsb;
	UINT8 m_i8751_ports[4];

	UINT16 *m_buffered_spriteram;
	UINT16 m_pri;
	DECLARE_WRITE16_MEMBER(dec0_control_w);
	DECLARE_WRITE16_MEMBER(slyspy_control_w);
	DECLARE_WRITE16_MEMBER(midres_sound_w);
	DECLARE_WRITE16_MEMBER(unmapped_w);
	DECLARE_READ16_MEMBER(slyspy_controls_r);
	DECLARE_READ16_MEMBER(slyspy_protection_r);
	DECLARE_WRITE16_MEMBER(slyspy_state_w);
	DECLARE_READ16_MEMBER(slyspy_state_r);
	DECLARE_READ16_MEMBER(dec0_controls_r);
	DECLARE_READ16_MEMBER(dec0_rotary_r);
	DECLARE_READ16_MEMBER(midres_controls_r);
	DECLARE_READ8_MEMBER(hippodrm_prot_r);
	DECLARE_WRITE8_MEMBER(hippodrm_prot_w);
	DECLARE_READ8_MEMBER(hippodrm_shared_r);
	DECLARE_WRITE8_MEMBER(hippodrm_shared_w);
	DECLARE_READ8_MEMBER(dec0_mcu_port_r);
	DECLARE_WRITE8_MEMBER(dec0_mcu_port_w);
	DECLARE_READ16_MEMBER(hippodrm_68000_share_r);
	DECLARE_WRITE16_MEMBER(hippodrm_68000_share_w);
	DECLARE_WRITE16_MEMBER(sprite_mirror_w);
	DECLARE_READ16_MEMBER(robocop_68000_share_r);
	DECLARE_WRITE16_MEMBER(robocop_68000_share_w);
	DECLARE_WRITE16_MEMBER(dec0_update_sprites_w);
	DECLARE_WRITE16_MEMBER(dec0_paletteram_rg_w);
	DECLARE_WRITE16_MEMBER(dec0_paletteram_b_w);
	DECLARE_WRITE16_MEMBER(dec0_priority_w);
	DECLARE_DRIVER_INIT(robocop);
	DECLARE_DRIVER_INIT(hippodrm);
	DECLARE_DRIVER_INIT(hbarrel);
	DECLARE_DRIVER_INIT(slyspy);
	DECLARE_DRIVER_INIT(birdtry);
	DECLARE_DRIVER_INIT(baddudes);
	DECLARE_DRIVER_INIT(midresb);
};


class dec0_automat_state : public dec0_state
{
public:
	dec0_automat_state(const machine_config &mconfig, device_type type, const char *tag)
		: dec0_state(mconfig, type, tag) {

	}

	UINT8 m_automat_adpcm_byte;
	int m_automat_msm5205_vclk_toggle;
	DECLARE_WRITE16_MEMBER(automat_control_w);
	DECLARE_WRITE8_MEMBER(automat_adpcm_w);
	DECLARE_READ16_MEMBER( automat_palette_r );
	DECLARE_WRITE16_MEMBER( automat_palette_w );
	DECLARE_WRITE16_MEMBER( automat_scroll_w )
	{
		COMBINE_DATA(&m_automat_scroll_regs[offset]);
	}
	UINT16 m_automat_scroll_regs[4];


};


/*----------- defined in video/dec0.c -----------*/

/* Video emulation definitions */
VIDEO_START( dec0 );
VIDEO_START( dec0_nodma );
VIDEO_START( automat );
SCREEN_UPDATE_IND16( hbarrel );
SCREEN_UPDATE_IND16( baddudes );
SCREEN_UPDATE_IND16( birdtry );
SCREEN_UPDATE_IND16( robocop );
SCREEN_UPDATE_IND16( hippodrm );
SCREEN_UPDATE_IND16( slyspy );
SCREEN_UPDATE_IND16( midres );

// bootlegs
SCREEN_UPDATE_IND16( automat );
SCREEN_UPDATE_IND16( secretab );

/*----------- defined in machine/dec0.c -----------*/

READ16_HANDLER( slyspy_controls_r );


extern void dec0_i8751_write(running_machine &machine, int data);
extern void dec0_i8751_reset(running_machine &machine);

