/*************************************************************************

    Irem M92 hardware

*************************************************************************/

#include "video/bufsprite.h"

struct pf_layer_info
{
	tilemap_t *		tmap;
	tilemap_t *		wide_tmap;
	UINT16			vram_base;
	UINT16			control[4];
};

class m92_state : public driver_device
{
public:
	m92_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	UINT16 m_sound_status;
	UINT8 m_irq_vectorbase;
	UINT32 m_raster_irq_position;
	UINT16 *m_vram_data;
	UINT16 m_videocontrol;
	UINT16 *m_spritecontrol;
	UINT8 m_sprite_buffer_busy;
	UINT8 m_game_kludge;
	pf_layer_info m_pf_layer[3];
	UINT16 m_pf_master_control[4];
	INT32 m_sprite_list;
	UINT8 m_palette_bank;

	required_device<buffered_spriteram16_device> m_spriteram;
	DECLARE_READ16_MEMBER(m92_eeprom_r);
	DECLARE_WRITE16_MEMBER(m92_eeprom_w);
	DECLARE_WRITE16_MEMBER(m92_coincounter_w);
	DECLARE_WRITE16_MEMBER(m92_bankswitch_w);
	DECLARE_WRITE16_MEMBER(m92_soundlatch_w);
	DECLARE_READ16_MEMBER(m92_sound_status_r);
	DECLARE_READ16_MEMBER(m92_soundlatch_r);
	DECLARE_WRITE16_MEMBER(m92_sound_irq_ack_w);
	DECLARE_WRITE16_MEMBER(m92_sound_status_w);
	DECLARE_WRITE16_MEMBER(m92_sound_reset_w);
	DECLARE_WRITE16_MEMBER(m92_spritecontrol_w);
	DECLARE_WRITE16_MEMBER(m92_videocontrol_w);
	DECLARE_READ16_MEMBER(m92_paletteram_r);
	DECLARE_WRITE16_MEMBER(m92_paletteram_w);
	DECLARE_WRITE16_MEMBER(m92_vram_w);
	DECLARE_WRITE16_MEMBER(m92_pf1_control_w);
	DECLARE_WRITE16_MEMBER(m92_pf2_control_w);
	DECLARE_WRITE16_MEMBER(m92_pf3_control_w);
	DECLARE_WRITE16_MEMBER(m92_master_control_w);
};


/*----------- defined in drivers/m92.c -----------*/

extern void m92_sprite_interrupt(running_machine &machine);


/*----------- defined in video/m92.c -----------*/


VIDEO_START( m92 );
SCREEN_UPDATE_IND16( m92 );
VIDEO_START( ppan );
SCREEN_UPDATE_IND16( ppan );
