
class fromanc2_state : public driver_device
{
public:
	fromanc2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16   *m_paletteram[2];
	UINT16   *m_videoram[2][4];

	/* video-related */
	tilemap_t  *m_tilemap[2][4];
	int      m_scrollx[2][4];
	int      m_scrolly[2][4];
	int      m_gfxbank[2][4];

	/* misc */
	int      m_portselect;
	UINT8    m_subcpu_int_flag;
	UINT8    m_subcpu_nmi_flag;
	UINT8    m_sndcpu_nmi_flag;
	UINT16   m_datalatch1;
	UINT8    m_datalatch_2h;
	UINT8    m_datalatch_2l;

	/* devices */
	device_t *m_audiocpu;
	device_t *m_subcpu;
	device_t *m_eeprom;
	device_t *m_left_screen;
	device_t *m_right_screen;
	DECLARE_WRITE16_MEMBER(fromanc2_sndcmd_w);
	DECLARE_WRITE16_MEMBER(fromanc2_portselect_w);
	DECLARE_READ16_MEMBER(fromanc2_keymatrix_r);
	DECLARE_WRITE16_MEMBER(fromanc2_eeprom_w);
	DECLARE_WRITE16_MEMBER(fromancr_eeprom_w);
	DECLARE_WRITE16_MEMBER(fromanc4_eeprom_w);
	DECLARE_WRITE16_MEMBER(fromanc2_subcpu_w);
	DECLARE_READ16_MEMBER(fromanc2_subcpu_r);
	DECLARE_READ8_MEMBER(fromanc2_maincpu_r_l);
	DECLARE_READ8_MEMBER(fromanc2_maincpu_r_h);
	DECLARE_WRITE8_MEMBER(fromanc2_maincpu_w_l);
	DECLARE_WRITE8_MEMBER(fromanc2_maincpu_w_h);
	DECLARE_WRITE8_MEMBER(fromanc2_subcpu_nmi_clr);
	DECLARE_READ8_MEMBER(fromanc2_sndcpu_nmi_clr);
	DECLARE_WRITE8_MEMBER(fromanc2_subcpu_rombank_w);
	DECLARE_READ16_MEMBER(fromanc2_paletteram_0_r);
	DECLARE_READ16_MEMBER(fromanc2_paletteram_1_r);
	DECLARE_WRITE16_MEMBER(fromanc2_paletteram_0_w);
	DECLARE_WRITE16_MEMBER(fromanc2_paletteram_1_w);
	DECLARE_READ16_MEMBER(fromancr_paletteram_0_r);
	DECLARE_READ16_MEMBER(fromancr_paletteram_1_r);
	DECLARE_WRITE16_MEMBER(fromancr_paletteram_0_w);
	DECLARE_WRITE16_MEMBER(fromancr_paletteram_1_w);
	DECLARE_READ16_MEMBER(fromanc4_paletteram_0_r);
	DECLARE_READ16_MEMBER(fromanc4_paletteram_1_r);
	DECLARE_WRITE16_MEMBER(fromanc4_paletteram_0_w);
	DECLARE_WRITE16_MEMBER(fromanc4_paletteram_1_w);
	DECLARE_WRITE16_MEMBER(fromanc2_videoram_0_w);
	DECLARE_WRITE16_MEMBER(fromanc2_videoram_1_w);
	DECLARE_WRITE16_MEMBER(fromanc2_videoram_2_w);
	DECLARE_WRITE16_MEMBER(fromanc2_videoram_3_w);
	DECLARE_WRITE16_MEMBER(fromanc2_gfxreg_0_w);
	DECLARE_WRITE16_MEMBER(fromanc2_gfxreg_1_w);
	DECLARE_WRITE16_MEMBER(fromanc2_gfxreg_2_w);
	DECLARE_WRITE16_MEMBER(fromanc2_gfxreg_3_w);
	DECLARE_WRITE16_MEMBER(fromanc2_gfxbank_0_w);
	DECLARE_WRITE16_MEMBER(fromanc2_gfxbank_1_w);
	DECLARE_WRITE16_MEMBER(fromancr_videoram_0_w);
	DECLARE_WRITE16_MEMBER(fromancr_videoram_1_w);
	DECLARE_WRITE16_MEMBER(fromancr_videoram_2_w);
	DECLARE_WRITE16_MEMBER(fromancr_gfxreg_0_w);
	DECLARE_WRITE16_MEMBER(fromancr_gfxreg_1_w);
	DECLARE_WRITE16_MEMBER(fromanc4_videoram_0_w);
	DECLARE_WRITE16_MEMBER(fromanc4_videoram_1_w);
	DECLARE_WRITE16_MEMBER(fromanc4_videoram_2_w);
	DECLARE_WRITE16_MEMBER(fromanc4_gfxreg_0_w);
	DECLARE_WRITE16_MEMBER(fromanc4_gfxreg_1_w);
	DECLARE_WRITE16_MEMBER(fromanc4_gfxreg_2_w);
};




/*----------- defined in video/fromanc2.c -----------*/

SCREEN_UPDATE_IND16( fromanc2_left );
SCREEN_UPDATE_IND16( fromanc2_right );
VIDEO_START( fromanc2 );
VIDEO_START( fromancr );
VIDEO_START( fromanc4 );

void fromancr_gfxbank_w(running_machine &machine, int data);
