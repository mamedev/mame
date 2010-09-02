class scotrsht_state : public driver_device
{
public:
	scotrsht_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int irq_enable;
	UINT8 *scroll;

	size_t spriteram_size;
	UINT8 *spriteram;
	UINT8 *videoram;
	UINT8 *colorram;
	tilemap_t *bg_tilemap;
	int charbank;
	int palette_bank;
};


/*----------- defined in video/scotrsht.c -----------*/

WRITE8_HANDLER( scotrsht_videoram_w );
WRITE8_HANDLER( scotrsht_colorram_w );
WRITE8_HANDLER( scotrsht_charbank_w );
WRITE8_HANDLER( scotrsht_palettebank_w );

PALETTE_INIT( scotrsht );
VIDEO_START( scotrsht );
VIDEO_UPDATE( scotrsht );
