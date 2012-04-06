/*************************************************************************

    Irem M107 hardware

*************************************************************************/


typedef struct _pf_layer_info pf_layer_info;
struct _pf_layer_info
{
	tilemap_t *		tmap;
	UINT16			vram_base;
	UINT16			control[4];
};

class m107_state : public driver_device
{
public:
	m107_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_irq_vectorbase;
	int m_sound_status;
	UINT16 *m_vram_data;
	UINT8 m_spritesystem;
	UINT8 m_sprite_display;
	UINT16 m_raster_irq_position;
	pf_layer_info m_pf_layer[4];
	UINT16 m_control[0x10];
	UINT16 *m_spriteram;
	UINT16 *m_buffered_spriteram;
	DECLARE_WRITE16_MEMBER(m107_coincounter_w);
	DECLARE_WRITE16_MEMBER(m107_bankswitch_w);
	DECLARE_WRITE16_MEMBER(m107_soundlatch_w);
	DECLARE_READ16_MEMBER(m107_sound_status_r);
	DECLARE_READ16_MEMBER(m107_soundlatch_r);
	DECLARE_WRITE16_MEMBER(m107_sound_irq_ack_w);
	DECLARE_WRITE16_MEMBER(m107_sound_status_w);
	DECLARE_WRITE16_MEMBER(m107_sound_reset_w);
	DECLARE_WRITE16_MEMBER(wpksoc_output_w);
	DECLARE_WRITE16_MEMBER(m107_vram_w);
	DECLARE_WRITE16_MEMBER(m107_control_w);
	DECLARE_WRITE16_MEMBER(m107_spritebuffer_w);
};


/*----------- defined in video/m107.c -----------*/

SCREEN_UPDATE_IND16( m107 );
VIDEO_START( m107 );
