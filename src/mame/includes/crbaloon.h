/*************************************************************************

Crazy Ballooon

*************************************************************************/


#define CRBALOON_MASTER_XTAL	(XTAL_9_987MHz)


class crbaloon_state : public driver_device
{
public:
	crbaloon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_pc3092_data;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_spriteram;
	UINT16 m_collision_address;
	UINT8 m_collision_address_clear;
	tilemap_t *m_bg_tilemap;
	UINT8 m_irq_mask;
	DECLARE_WRITE8_MEMBER(pc3092_w);
	DECLARE_READ8_MEMBER(pc3259_r);
	DECLARE_WRITE8_MEMBER(port_sound_w);
	DECLARE_WRITE8_MEMBER(crbaloon_videoram_w);
	DECLARE_WRITE8_MEMBER(crbaloon_colorram_w);
};


/*----------- defined in audio/crbaloon.c -----------*/

WRITE8_DEVICE_HANDLER( crbaloon_audio_set_music_freq );
WRITE8_DEVICE_HANDLER( crbaloon_audio_set_music_enable );
void crbaloon_audio_set_explosion_enable(device_t *sn, int enabled);
void crbaloon_audio_set_breath_enable(device_t *sn, int enabled);
void crbaloon_audio_set_appear_enable(device_t *sn, int enabled);
WRITE8_DEVICE_HANDLER( crbaloon_audio_set_laugh_enable );

MACHINE_CONFIG_EXTERN( crbaloon_audio );


/*----------- defined in video/crbaloon.c -----------*/

PALETTE_INIT( crbaloon );
VIDEO_START( crbaloon );
SCREEN_UPDATE_IND16( crbaloon );


UINT16 crbaloon_get_collision_address(running_machine &machine);
void crbaloon_set_clear_collision_address(running_machine &machine, int _crbaloon_collision_address_clear);
