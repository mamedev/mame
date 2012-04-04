class scotrsht_state : public driver_device
{
public:
	scotrsht_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_irq_enable;
	UINT8 *m_scroll;

	size_t m_spriteram_size;
	UINT8 *m_spriteram;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	tilemap_t *m_bg_tilemap;
	int m_charbank;
	int m_palette_bank;
	DECLARE_WRITE8_MEMBER(ctrl_w);
	DECLARE_WRITE8_MEMBER(scotrsht_soundlatch_w);
};


/*----------- defined in video/scotrsht.c -----------*/

WRITE8_HANDLER( scotrsht_videoram_w );
WRITE8_HANDLER( scotrsht_colorram_w );
WRITE8_HANDLER( scotrsht_charbank_w );
WRITE8_HANDLER( scotrsht_palettebank_w );

PALETTE_INIT( scotrsht );
VIDEO_START( scotrsht );
SCREEN_UPDATE_IND16( scotrsht );
