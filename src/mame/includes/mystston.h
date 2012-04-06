/***************************************************************************

    Technos Mysterious Stones hardware

    driver by Nicola Salmoria

***************************************************************************/


#define	MYSTSTON_MASTER_CLOCK	(XTAL_12MHz)


class mystston_state : public driver_device
{
public:
	mystston_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* machine state */
	UINT8 *m_ay8910_data;
	UINT8 *m_ay8910_select;

	/* video state */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	emu_timer *m_interrupt_timer;
	UINT8 *m_bg_videoram;
	UINT8 *m_fg_videoram;
	UINT8 *m_spriteram;
	UINT8 *m_paletteram;
	UINT8 *m_scroll;
	UINT8 *m_video_control;
	DECLARE_WRITE8_MEMBER(irq_clear_w);
	DECLARE_WRITE8_MEMBER(mystston_ay8910_select_w);
	DECLARE_WRITE8_MEMBER(mystston_video_control_w);
};


/*----------- defined in drivers/mystston.c -----------*/

void mystston_on_scanline_interrupt(running_machine &machine);


/*----------- defined in video/mystston.c -----------*/

MACHINE_CONFIG_EXTERN( mystston_video );
